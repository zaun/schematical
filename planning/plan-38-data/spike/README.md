# plan-38 routing spike (2026-11-13, throwaway)

What this is: a prototype grid router, written to measure how hard the real
boards are before the plan fixed a design. Not product code; nothing here is
built by the app.

- `DumpScene.swift.txt` — an XCTest (was `ZZRouteProbeTests.swift`, run in a
  detached worktree at cb6259f, then deleted) that opens each fixture with
  `SchematicFile.decode(...).apply(...)` and writes pads (rect, drill, plated,
  side, net, connectivity root), outline, rules and the unrouted count to JSON.
- `router-v1-no-ripup.swift` — single-pass A* (8 directions, via cost 12,
  bend cost 0.3, hard-blocked obstacles stamped on a grid).
  Build: `swiftc -O router-v1-no-ripup.swift -o router1`.
  Run: `router1 <AFU|LFU|RegFile> <pitch mm> <trace width mm> <short|long|fewest>`.
- `router-v2-ripup.swift` — v1 plus whole-net rip-up and reroute (a failed
  connection is searched again with foreign copper passable at cost 25; the nets
  it crosses are ripped up and queued again; at most 12 rip-ups per net).
  Run the same way; the order also takes `big`.
- `fixtures/` (one level up) — the three boards exactly as measured.

Both prototypes iterate Swift dictionaries, whose order is random per process,
so repeated runs differ. `results.txt` lists every run.
