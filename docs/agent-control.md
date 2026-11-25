# Agent control (MCP)

Schematical can be driven by an AI agent (Claude Code, or any MCP client)
while it runs: the agent searches parts, places and wires them, lays out and
routes the board, runs the design rule check and the simulation, and looks at
pictures of the result. The windows redraw live and every edit is undoable.
The agent works through the same `SchematicDocument` methods the canvas
calls, so it cannot make a design the app can't.

Code: `schematical/schematical/Automation/`. Tests:
`schematicalTests/AutomationTests.swift`. Background and the options
considered: `planning/spike-agent-control.md`.

## Turning it on

Settings ▸ **AI Agents**: the toggle starts and stops the server at once (and
is read at launch). It is off by default. The page shows the address, a key,
the connection status, and a ready-made command:

```
claude mcp add --transport http schematical http://127.0.0.1:47800/mcp \
  --header "Authorization: Bearer <key>"
```

Other clients: a Streamable HTTP MCP server at `http://127.0.0.1:47800/mcp`
with the header `Authorization: Bearer <key>`. **New Key** replaces the key
(agents using the old one are refused).

Files: the app is sandboxed, so `open`, `save`, `export_gerber` and `export_pdf` work only
inside one folder the user shares under Settings ▸ AI Agents ▸ Files
(`AutomationFolder`, a security-scoped bookmark). With none chosen those
tools say so; nothing else depends on it.

## Safety

- Loopback only: the listener is bound to the `127.0.0.1` endpoint. Setting
  `requiredInterfaceType = .loopback` is not enough: it still binds `*`.
- Every request needs the bearer key (constant-time compare); the `Host`
  header must be loopback; a request with an `Origin` header (a web page) is
  refused with 403; request bodies are capped at 8 MB.
- Off unless enabled. Settings shows which agent last connected and how many
  tool calls it made.
- The key lives in the app's preferences. Any process running as the user can
  read it: the key stops web pages and other users, not malware as you.
- The design is read-only while the simulation runs (`simulation_stop`
  first), as in the app.
- Files stay inside the shared folder (symlinks and `..` are resolved before
  the check).
- No tool quits the app or touches anything outside the open designs and the
  shared folder.

## The tools (53)

Every tool except the few marked *(no window)* takes an optional `window`
(an id from `list_windows`; the frontmost window by default).

**Read** — `get_design` (text, or `format=json`; `geometry=true` adds pin tips), `get_nets`,
`search_parts`*(no window)*, `get_part`*(no window)*, `get_selection`,
`get_board`, `run_erc` (the electrical rules check of the schematic, by KiCad's
rules, one violation per line: `<n>. [error|warning] [rule] <reference>: <message>`;
it honours the file's severities and exclusions, as Window › Electrical Rules
Check does), `run_drc`, `render` (schematic, pcb, front, back; `region`
crops to zoom in). `get_design` ends with a `DOCUMENT` section (JSON: `"document"`):
title, author, revision, description, created and updated (ISO 8601), whether the dates are automatic,
and the title block's place and bounds (`title_block` is `null` when it is off).

A pin's active-low flag (the part file's `"activeLow"`, `docs/parts-format.md`
§ Pinouts and pins) is in `get_design`'s JSON: a pin dictionary has
`"activeLow": true` when its whole name is active low, or the list of its
active-low name alternatives (`["CS"]` for `A0/CS`), and no key otherwise;
the text form is unchanged. `get_part` prints it after a pin's role and
group: ` low`, or ` low=CS` (alternatives `;` separated).

**Schematic** — `place_part`, `connect` (pin, net label, bus pin or point on
a wire; `via` waypoints), `disconnect`, `move_part`, `move`, `flip_part`, `rotate_part` (parts drawn as their own symbol: LEDs, diodes, resistors, capacitors, crystals, inductors, fuses, buzzers, transistors, test points, barrel jacks), `set_part`
(rename, color, ohms, farads, footprint, gender; `socket_mode` `ic`, `ic+socket` or `socket-only` and `socket`,
the socket's part name, for a DIP or PLCC chip a socket fits: a mode alone picks the generic socket, and `get_design` shows
`socket=<name> (<mode>)` on the part), `add_net_label`,
`add_note`, `set_net_label`, `set_note`, `set_document_info` (title, author, revision, description, `created`/`updated`
(ISO 8601, only with `automatic_dates` false: saves set them while it is on), `title_block` true/false to show or hide the
title block), `set_wire_route`, `add_bus`, `set_bus`, `add_bus_port`, `set_bus_port`, `delete`, `select`.

**Gate view** (plan-67). A part with units (a gate IC, Logic → Gates) is drawn as one block or as its
units: `set_part reference=U1 view=gate` (back: `view=block`; `place_part` takes `view` too, in the same
undo step). Its units are then addressed `U1-A`, `U1-B`, … (the gates, in the order of the part's gates)
and `U1-P` (the block of the pins no gate owns: power, ground, NC). `move_part` moves one unit (`U1-A`)
or the whole part (`U1`); `flip_part` and `rotate_part` take a unit (the `P` block flips but does not
turn; a whole part in Gate view is refused with the list of its units); `select U1-A` selects the part
with that unit picked, and `get_selection` then prints `U1 (U1-A)`. A pin is still `U1.3`; `U1-A.3` is
accepted too and refused when the pin is not that unit's (`U1-A has no pin 6; its pins: 1=1A 2=1B 3=1Y`).
`delete U1-A` is refused (`U1-A is one symbol of U1; delete U1 to remove the part`); `delete U1` removes
the whole part without the question the app asks a person. `get_design` shows `view=gate` on the part's
line and one indented line per unit (`U1-A centre=… size=… body=… [flipped] [rotation=90] pins=1,2,3`);
its JSON has `"view"` (`"gate"` or `"block"`, on every part that has units) and, for a part in Gate view,
`"units"`: `unit`, `reference`, `x`, `y` (centre), `width`, `height`, `flipped`, `rotation`, `bounds`
and `pins` (designators). The part's own `x`, `y`, `width`, `height` and `bounds` are then the box around
all its units, and each pin's tip is where its unit draws it. Nets, ERC, DRC and every PCB tool keep
writing `U1.3`: the part is one part.

**PCB** — `set_board` (size, type, layers, grid, units, color, `outline` for a free-form edge, `rectangle` to take it off), `set_layer`
(plane or wire layers, plane net, clearance), `move_footprint` (position,
rotation, side), `route` (pad to pad, with warnings when it crosses another
pad), `add_trace`, `set_trace`, `add_via`, `add_hole`, `add_pour` (flood a rectangle with
copper on one net, thermal reliefs or solid), `set_pour`, `add_keepout` (a rectangle the pours stay
out of), `set_keepout`, `add_cutout` (a rectangle, slot, circle or polygon cut out of the board; copper keeps its
distance from its edge), `set_cutout`, `add_board_text`, `lock`. `get_board` lists `POURS`, `KEEPOUTS` and `CUTOUTS`,
and marks a free-form board `outline=free-form`.

**Simulation** — `simulation_start`, `simulation_stop`, `simulation_status`,
`wait`*(no window)*, `set_switch` (a DIP position on or off; a tactile button pressed or released; a slide switch's knob moved to `position` 1 … N, `on` not used), `read_pins` (levels and LED brightness),
`read_memory`.

**Session** — `list_windows`*(no window)*, `focus_window`, `new_window`,
`open`, `save`, `export_gerber`, `export_pdf`, `list_files`*(no window)*, `undo`, `redo`,
`batch`.

A window is a document (`SchematicNSDocument`): `list_windows` lists the
open documents, with `unsaved` on one whose changes are not in its file
(a schematic with a file saves itself as it is edited, so this is mostly
one never saved). `new_window` makes a new untitled document and `open`
opens a file as the app's File › Open does; neither needs a window to be
open, and a file that is already open is not opened twice (`open` names
its window). `save` with no path saves the document to its own file, and
fails for one that has none (`This window has no file yet; pass a path.`).
`save` with a path is a **Save As**: the file is written there and the
window is that file's from then on.

`undo` and `redo` act on the document: they undo or redo its last edit,
whichever page made it, and take no page.

`batch` runs a list of tool calls as **one undo step** (it wraps them in the
model's gesture), stops at the first error and reports which call failed.

Not there yet: autorouting (plan-38's router isn't in the app), net classes
and differential pairs, copy/paste, custom parts, the part editor.

## Conventions an agent needs

The server sends these as MCP `instructions` on connect
(`AutomationGuide.text`); keep the two in step.

- Parts by reference (`U1`), pins by designator or name (`U1.14`, `D1.K`).
  A gate IC can be drawn as separate gates: `set_part` `view=gate`. Its gates are then `U1-A`, `U1-B`, …,
  and `U1-P` is the block with its power pins; move, flip and rotate them one at a time (`move_part`,
  `flip_part`, `rotate_part`). Pins are still `U1.3` (`U1-A.3` also works). Deleting takes the whole part.
  Wires, labels, notes, traces, vias, holes, pours (`pour:ID`), keep-outs (`keepout:ID`), cutouts (`cutout:ID`), texts by the 8-hex-digit id the
  read tools print. The Document Info title block is the item `titleblock` (for `select`, `move`, `delete`; `delete`
  only hides it, and it is an error while it is off: `set_document_info title_block=true` shows it).
- Schematic: canvas units, y down, 10-unit grid, parts placed by centre.
  Wires take the shortest right-angle route and do not avoid parts: pass
  `via` waypoints, and `render` to check.
- PCB: millimetres, y down, origin at the board's top-left; footprints snap
  to the board grid. New footprints are auto-placed in a row, usually off the
  board: lay them out with `move_footprint`.
- Power symbols (`GND`, `5V`, `3V3`, …, `PULLDOWN`, `PULLUP-5V`) are parts
  placed by name; `search_parts` doesn't list them.

## Code map

| File | What |
|---|---|
| `AutomationServer.swift` | Listener, HTTP framing, MCP JSON-RPC, key, request checks, status |
| `AutomationTools.swift` | Tool type, argument readers, window lookup, dispatch, the guide |
| `AutomationRead.swift`, `AutomationSchematic.swift`, `AutomationBoard.swift`, `AutomationSimulation.swift`, `AutomationSession.swift` | The tools, by area |
| `AutomationFolder.swift` | The shared folder and path containment |
| `Preferences/PreferencesView.swift` | Settings ▸ AI Agents |

Hooks outside that folder: the open documents are
`NSDocumentController.shared.documents`, each a `SchematicNSDocument` with
its `SchematicEditorModel` (document, simulation, exporter); `new_window`,
`open` and `save` call `NSDocumentController` and `NSDocument.save(to:ofType:for:)`;
`SchematicExporter` writes the exports; `SimulationController.level(ofPin:on:)`;
`PartCatalogStore.allParts`; the build setting
`ENABLE_INCOMING_NETWORK_CONNECTIONS = YES` (the sandbox's server
entitlement).

## Adding a tool

Add an `AutomationTool` to the right `…Tools` array: a name, a summary the
agent reads, a JSON-schema `properties` dictionary (`Schema.*`), and a `run`
closure that returns `Out.text(…)`, `Out.json(…)` or `Out.image(…)`. Throw
`AutomationFailure("what is wrong, and the valid choices")` for a refusal.
Set `edits: true` for anything that changes the design (refused while the
simulation runs) and `readOnly: true` for readers. Prefer a method the
canvas already calls (so undo, voltage checks and selection behave) and name
things the way an agent sees them. Add a test in `AutomationTests`, and
mention it in `AutomationGuide` only if it changes how an agent should work.
