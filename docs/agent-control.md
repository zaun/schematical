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
`get_board`, `run_drc`, `render` (schematic, pcb, front, back; `region`
crops to zoom in).

A pin's active-low flag (the part file's `"activeLow"`, `docs/parts-format.md`
§ Pinouts and pins) is in `get_design`'s JSON: a pin dictionary has
`"activeLow": true` when its whole name is active low, or the list of its
active-low name alternatives (`["CS"]` for `A0/CS`), and no key otherwise;
the text form is unchanged. `get_part` prints it after a pin's role and
group: ` low`, or ` low=CS` (alternatives `;` separated).

**Schematic** — `place_part`, `connect` (pin, net label, bus pin or point on
a wire; `via` waypoints), `disconnect`, `move_part`, `move`, `flip_part`, `rotate_part` (LEDs, diodes, resistors, capacitors), `set_part`
(rename, color, ohms, farads, footprint, gender; `socket_mode` `ic`, `ic+socket` or `socket-only` and `socket`,
the socket's part name, for a DIP or PLCC chip a socket fits: a mode alone picks the generic socket, and `get_design` shows
`socket=<name> (<mode>)` on the part), `add_net_label`,
`add_note`, `set_net_label`, `set_note`, `set_wire_route`, `add_bus`, `set_bus`, `add_bus_port`, `set_bus_port`, `delete`, `select`.

**PCB** — `set_board` (size, type, layers, grid, units, color), `set_layer`
(plane or wire layers, plane net, clearance), `move_footprint` (position,
rotation, side), `route` (pad to pad, with warnings when it crosses another
pad), `add_trace`, `set_trace`, `add_via`, `add_hole`, `add_pour` (flood a rectangle with
copper on one net, thermal reliefs or solid), `set_pour`, `add_keepout` (a rectangle the pours stay
out of), `set_keepout`, `add_board_text`, `lock`. `get_board` lists `POURS` and `KEEPOUTS`.

**Simulation** — `simulation_start`, `simulation_stop`, `simulation_status`,
`wait`*(no window)*, `set_switch` (a DIP position on or off; a tactile button pressed or released; a slide switch's knob moved to `position` 1 … N, `on` not used), `read_pins` (levels and LED brightness),
`read_memory`.

**Session** — `list_windows`*(no window)*, `focus_window`, `new_window`,
`open`, `save`, `export_gerber`, `export_pdf`, `list_files`*(no window)*, `undo`, `redo`,
`batch`.

`batch` runs a list of tool calls as **one undo step** (it wraps them in the
model's gesture), stops at the first error and reports which call failed.

Not there yet: autorouting (plan-38's router isn't in the app), net classes
and differential pairs, copy/paste, custom parts, the part editor.

## Conventions an agent needs

The server sends these as MCP `instructions` on connect
(`AutomationGuide.text`); keep the two in step.

- Parts by reference (`U1`), pins by designator or name (`U1.14`, `D1.K`).
  Wires, labels, notes, traces, vias, holes, pours (`pour:ID`), keep-outs (`keepout:ID`), texts by the 8-hex-digit id the
  read tools print.
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

Hooks outside that folder: `SchematicSessions` (Memory/MemoryWindow.swift)
lists windows with their file controller; `SchematicFileController` has
`open(_:)` and `saveWithoutPanel(to:)`; `SimulationController.level(ofPin:on:)`;
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
