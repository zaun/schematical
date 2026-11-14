# Spike: AI agents controlling the running app (MCP)

> **Status: built.** The v1 described under "Proposed tool surface" is
> implemented; see `docs/agent-control.md` for what ships. This file keeps the
> research, the options considered and the spike's findings.

Goal: let an agent (Claude Code, etc.) build schematics and PCBs in the
**running app**, with the window updating live, not by editing the
`.schematical` file.

## Verdict

**Embed a small MCP server in the app** (Streamable HTTP on 127.0.0.1) whose
tools call the same `SchematicDocument` methods the canvas calls. The spike
proves it works end to end, in the sandboxed, signed build, with Claude Code
as the client. Cost of the prototype: ~450 lines in two new files plus four
one-line hooks. The model layer was already clean enough that no refactor was
needed.

## Options considered

| Option | Verdict |
|---|---|
| Agent edits the `.schematical` JSON | Works today, but no live window, no undo, no validation, the app doesn't reload on change, and the agent never sees the result. This is what the user wants to get away from. |
| AppleScript / `.sdef` scripting | Poor fit for SwiftUI `WindowGroup`, verbose, hard for agents to discover; no images. |
| App Intents / Shortcuts | Coarse, user-facing actions; no good way to expose a rich edit API. |
| Accessibility / computer-use driving the GUI | Slow, fragile, coordinate-based. Keep only for testing the GUI itself. |
| **Embedded MCP server (HTTP)** | **Chosen.** Direct, typed, one undo step per call, can return images. No extra process. |
| Separate stdio MCP binary talking to the app over a unix socket | More moving parts (second artifact, discovery, install). Only worth it if some client can't speak HTTP MCP; Claude Code can. Revisit later; the tool layer is transport-independent. |

## What was built (uncommitted, on `main`)

- `schematical/schematical/Automation/AutomationServer.swift`: `NWListener`
  HTTP server, JSON-RPC `initialize` / `ping` / `tools/list` / `tools/call`.
  Off until Settings ▸ AI Agents turns it on (a description and an on/off
  toggle; it shows the address and the `claude mcp add` command while on).
  Fixed port 47800. The toggle starts and stops the listener live.
- `schematical/schematical/Automation/AutomationTools.swift`: 11 tools:
  `list_windows`, `search_parts`, `get_design`, `place_part`, `connect`,
  `move_footprint`, `add_trace`, `run_drc`, `render`, `undo`, `redo`.
- Hooks: `schematicalApp.init` starts the server;
  `SchematicSessions.identifiedSessions` (Memory/MemoryWindow.swift) exposes
  window ids (the registry already existed for the BOM/DRC/memory windows);
  `PartCatalogStore.allParts`; `ENABLE_INCOMING_NETWORK_CONNECTIONS = YES` in
  both build configs (signed app now carries `com.apple.security.network.server`).

## What was verified (real app, signed + sandboxed Debug build)

- `initialize`, `tools/list`, `tools/call` over raw curl.
- Claude Code connected with `--mcp-config` (`type: http`), listed the tools
  and read the live design (`get_design`) of an open window.
- `place_part` → `connect` → `get_design`: parts get the normal references
  (D1, D2), pins resolve by name or designator (`D1.K`, `D1.1`), wires appear.
- Bad input returns readable errors (`no part Z9`; the pin-not-found error
  lists the part's real pins), so an agent can self-correct.
- `render` returns the schematic and PCB page as PNG (MCP image content),
  reusing File ▸ Export ▸ Image's `ImageExport.data`. An agent can look at its
  own work.
- `run_drc` returns the real design-rule results (ratsnest "unrouted",
  off-board, silkscreen).
- `undo` on each page reverses agent edits exactly as for a human edit
  (they go through `recordEdit`).
- Origin header → 403.
- The Settings page builds and the server is off at launch (no listener).
  The toggle itself was not exercised on screen (screen capture was black
  in the session), so check it by hand: on → `lsof -iTCP:47800` shows
  `localhost` LISTEN; off → nothing.

## Findings and gotchas

1. **`requiredInterfaceType = .loopback` does NOT bind loopback.** `lsof`
   showed `*:47800`, reachable from the LAN. Fixed with
   `requiredLocalEndpoint = 127.0.0.1:port` (`localhost:47800 (LISTEN)`).
   Keep a test for this.
2. **Sandbox:** needs the incoming-network entitlement (done). Outgoing is
   already on. Non-sandboxed/unsigned builds hide this problem.
3. **Window addressing:** each window has its own document (`@State` in
   `ContentView`); tabs make several. Needs stable ids, plus "frontmost
   window" as the default. The spike required an explicit id once more than
   one existed (it restored 3 blank windows). Catalog tools (`search_parts`)
   must not require a window.
4. **Raw coordinates are the wrong agent interface.** The spike's
   `add_trace` took mm points; the traces missed the pads (footprints
   snap to the 0.635 mm grid, so 20,20 → 19.685). Real PCB tools must address
   **pads and nets**: `route(from:"D1.1", to:"D2.2")`, `move_footprint`
   relative or by pad, snapped results returned. Same for schematic: place by
   position, wire by pin names (done), never by tip coordinates.
5. **Schematic render at scale 1 is too small** to read (~100 px). Auto-fit,
   a minimum width and a crop/zoom parameter are needed. The PCB page
   render is fine.
6. **Edits are not coalesced.** One tool call = one undo step; an agent
   making 60 calls makes 60 undo steps. Want a `begin_batch` / `end_batch`
   (the model has `beginGesture`/`endGesture`) so "undo" reverts one agent
   action group.
7. **Main-thread contention:** every call hops to the main actor, so a long
   simulation step or an autoroute blocks replies and the UI. Long tools
   (autoroute, simulation run) need to be async with a progress/cancel
   handle.
8. **No resistor kind in the catalog yet** (plan-34 pending), so the spike
   used LEDs; `search_parts` is a name/description substring match: needs
   category/package/pin-count filters and a `get_part` (pins, footprints)
   tool.
9. The unsaved-changes machinery (`hasUnsavedChanges`) is a full
   `SchematicFile` diff, so agent edits mark the window dirty with no extra
   work. Save/Open need tools (`save(path)`), but the sandbox only allows
   user-selected paths: see Security.
10. Not yet tried: Claude Desktop (needs `mcp-remote`/stdio shim for HTTP on
    some versions), Codex, Cursor. Only Claude Code was tested.

## Security

- Loopback bind (fixed), Origin rejected (stops a web page driving the app
  via DNS rebinding / CSRF). Must also: validate `Host` is
  `127.0.0.1`/`localhost`, require a **bearer token** (generated per launch,
  shown in Settings, copy-able as the `claude mcp add` command), and be
  **opt-in** in Settings ▸ Automation with a visible "agent connected"
  indicator.
- Any local process (including other users' apps) can reach loopback;
  the token is what separates them.
- File tools: only paths the user picked, or a dedicated folder; otherwise
  the sandbox blocks them anyway. Prefer `export(view)` returning bytes over
  file paths.
- No tool should be able to quit, delete files outside the container, or
  run the simulation unbounded.

## Proposed tool surface (v1)

Read: `list_windows`, `get_design` (structured JSON + text), `get_nets`
(netlist by name), `get_part` / `search_parts` (filters), `get_selection`,
`get_board` (outline, layers, rules), `run_drc`, `render` (view, region,
zoom).
Schematic edit: `place_part` (+ options: LED color, ohms, farads, footprint),
`connect`, `disconnect`, `move_part`, `rotate/flip`, `delete`, `add_net_label`,
`add_note`, `add_bus`, `set_value`.
PCB edit: `set_board`(size, layers, outline), `move_footprint`, `rotate`,
`set_side`, `route` (pad→pad, layer, width), `add_via`, `delete_trace`,
`autoroute` (when plan-38 lands), `add_zone/plane`, `lock`.
Session: `batch(begin/end)`, `undo/redo`, `select`, `focus_window`,
`new_window`, `open(path)`, `save`, `export_gerber`, `run_simulation` /
`set_input` / `read_pin` (the simulator makes this uniquely powerful: an
agent can design, simulate, and check).

Also ship an **MCP resource/prompt or `AGENTS`-style guide** describing
conventions (grid, units, coordinate systems, reference naming), since the
agent has no way to infer them.

## Recommended plan shape (for `write-plan`)

A. Server core: transport hardening (token, Host check, Settings toggle,
   status), window registry + "frontmost", tests for loopback bind. 
B. Read tools + structured design/netlist output + render improvements.
C. Schematic edit tools + batching/undo grouping.
D. PCB tools addressed by pad/net; depends on plan-38 for `autoroute`.
E. Simulation tools.
F. Docs + `claude mcp add` snippet + an agent guide; a test harness that
   drives the tools headlessly (the existing `scripts/harness` already
   builds the app sources into a CLI, so tool handlers can be unit-tested
   without a window).

## To reproduce the spike

```
xcodebuild -project schematical/schematical.xcodeproj -scheme schematical \
  -configuration Debug -derivedDataPath /tmp/spike-dd build
open /tmp/spike-dd/Build/Products/Debug/schematical.app   # then Settings ▸ AI Agents ▸ on
claude mcp add --transport http schematical http://127.0.0.1:47800/mcp
```

(Historical: the spike code had no key, no tests and a hand-written HTTP parser. v1 added the key,
Host/Origin checks, a body cap, tests and the Settings page; the HTTP parser is still hand-written.)
