# Routing

The PCB page routes copper for you: **Route ▸ Autoroute…** routes the whole
board (or a selection) with the options you pick, the **Route** tool routes
one net or one unrouted line with a click, and the **Trace** tool's
**Assist** makes a hand-drawn trace walk around other nets' copper. All of
it obeys the board's design rules: what the router puts down passes the
Design Rule Check (⇧⌘D) for the kinds it is responsible for, or it isn't
put down. The rules it follows per net come from **net classes**, **layer
rules** and **net groups**, set in the PCB sidebar's Nets, Net Classes and
Net Groups sections (nothing selected) and saved in the file
([file-format.md § Net classes](file-format.md#net-classes),
[§ Net layer rules](file-format.md#net-layer-rules),
[§ Net groups](file-format.md#net-groups)).

Plan: `planning/completed/plan-38-*.md`; the prototype it was measured
against and the three fixture boards: `planning/plan-38-data/`.

## Commands

| Command | Key | What it does |
|---|---|---|
| Route ▸ Autoroute… | ⌥⌘R | The Autoroute sheet: options, then Route. |
| Route ▸ Route Selected Nets | ⌥⇧⌘R | Routes the selection's nets with the last options, no sheet. |
| Route ▸ Unroute Selected | ⌥⌘U | Removes the selection's nets' unlocked traces and vias. |
| Route ▸ Unroute All Autorouted | — | Removes every unlocked trace and via the router put down (`auto`). |
| Route ▸ Select Autorouted Copper | — | Selects every trace and via the router put down. |
| Route tool (PCB toolbar) | — | Click a pad: route its net. Click an unrouted (ratsnest) line: route that one connection. Uses the last options. |
| Nets row ▸ Route Net / Unroute Net | — | The same for one net, from the sidebar's context menu. |
| Trace tool Assist (wand) | — | A dragged trace walks around obstacles; red where no legal way exists. |
| V while drawing a trace | V | Drops a via where the trace has got to and goes on on the next trace layer. |

The Route menu and the toolbar's Autoroute button are on the PCB page only,
and not while the simulation runs.

### The Autoroute sheet

The sheet shows how many nets and connections the options would route,
then while running a progress bar ("83 of 115 connections routed · GND")
with **Stop** and **Cancel**, and the board behind it draws the copper so
far at 60 %. **Stop** keeps what is routed (a legal partial result);
**Cancel** puts nothing down. When it finishes it lists each net still
unrouted with how many connections and why ("no legal way found", "run
stopped first", an impossible net's reason, a pair's or group's outcome),
and a **Design Rule Check** button. Clicking a listed net highlights it.
The whole run is one undo step. If the board changes while it runs, the
result is discarded ("The board changed while routing").

Every option is `RoutingOptions` (`Routing/RoutingOptions.swift`); all but
`scope` are remembered between runs (user defaults, key `autorouteOptions`).

| Option (`field`) | Choices | Default | Meaning |
|---|---|---|---|
| Nets (`scope`) | All nets; Selection | All nets (the selection when something is selected) | Which nets: all, some by name (`.nets`), those on the selected parts' pads (`.footprints`), or one unrouted line (`.connection`, the Route tool). |
| Which (`include`) | Every incomplete net; Only untouched nets | Every incomplete net | A net with 2 or more copper groups is routed; "untouched" also needs it to have no trace or via yet. |
| Existing (`existing`) | Keep all; Clear autorouted; Clear unlocked | Keep all | What happens to the scope's copper first. Locked copper always stays. |
| Layers (`layers`) | per layer: on/off, Any / Horizontal / Vertical | all on, Any | Which trace layers the run may use, and each layer's favoured direction (Alternate Directions sets Top horizontal, the next vertical, …). |
| Vias (`vias`) | Allowed; None | Allowed | None keeps every net on one layer. |
| Angles (`angle`) | 0°, 45° and 90°; 0° and 90° only | 0°, 45° and 90° (octilinear) | The segments' directions. |
| Strategy (`strategy`) | Fast; Balanced; Thorough | Balanced | How hard negotiation tries: Fast 6 rounds, rip-up limit 8 per connection; Balanced 30 rounds, 12; Thorough 120 rounds, 12, three seeds, the best kept. |
| Grid (`grid`) | 0.05, 0.1, 0.127, 0.254 mm | 0.127 mm (5 mil) | The search grid; 0.127 divides the 0.635 mm component grid, so pad centres and the channels between DIP pads fall on cells. |
| Width (`widthOverride`) | mil, or off | off | Every net this width instead of its class's. |
| Clearance (`clearanceOverride`) | mm, or off | off | Every net this clearance instead of its class's (never under the DRC's). |
| Fan out to Planes (`fanoutPlanes`) | on/off | on | A pad of a net that a Plane layer carries gets a short trace and a via to the plane instead of a route. |
| Tune length groups (`tuneGroups`) | on/off | on | Length-match groups are tuned after routing. |
| Route pairs side by side (`routePairs`) | on/off | on | Differential pairs are routed coupled. |
| — (`seed`) | integer, not in the sheet | 1 | The first run keeps the nets in id order; each further Thorough run (seed + 1, + 2) shuffles ties by it. Same seed and board, same copper. |
| Time limit (`timeLimit`) | seconds (60 when switched on), or off | off | Stop on its own after this long (as Stop). |

## Rules per net

- **Net classes.** A net's class gives its trace width (mil), clearance,
  via drill and diameter, and optionally a layer rule. A net is in the
  first class any of whose members names it; others are in the Default
  class (10 mil, the DRC's min clearance, a 0.4 / 0.8 mm via). The router
  routes each net at its class's width and keeps its class's clearance
  from every other net's copper (the larger of the two nets' clearances).
  The Trace tool and the Via tool use the same class values for what you
  draw. Net Classes ▸ Add Power Class makes a 20 mil class holding the
  supply and ground nets.
- **Layer rules.** "Only" holds a net to some layers (on one layer it gets
  no vias); "Prefer" lets it use any but makes the others dearer. A net's
  own rule comes before its class's. A pad off a net's only layers makes
  the net impossible (reported, not routed). The DRC reports copper off
  its layers (`layerRule`).
- **Length-match groups.** Two-pin nets that should end within `tolerance`
  of each other. Length is measured pad centre to pad centre along the
  copper (`RoutingLength`). After routing, the shorter nets get bumps
  (`Meander`, at most `maxMeander` across) where they fit, longest first
  untouched; the outcome is `matched`, or `residual` with the spread left.
  The DRC reports a group whose spread exceeds its tolerance
  (`lengthMatch`), and a member with more than two pins.
- **Differential pairs.** Two two-pin nets routed as one fat centreline
  (both traces plus the gap) on a grid where both nets' copper counts as
  one net's, then offset to two traces at `gap` (mitred at corners, so the
  gap holds), a flared via pair at each layer change, a short breakout leg
  from each lane to its pad, and checked whole. Skew is tuned on the
  outer side. A pair that can't be coupled is routed as two ordinary nets
  and reported `uncoupled` with the reason; it is never left half-done. The
  DRC reports a pair whose nets run apart for more than `maxUncoupled`
  (`diffPair`). A pair couples when each end's two pads stand side by side
  across the direction it leaves, with nothing between them (an adjacent
  pin pair of a connector); when another net's pad lies between them, or
  the two pads lie in line with the travel, the short breakout can't reach
  and the pair is uncoupled ("no legal breakout to a pad").
- **`auto`.** Copper the router puts down is marked `auto` in the file, so
  Clear autorouted and Unroute All Autorouted take it away and leave your
  own traces. Lock Position on a trace or via keeps it through every clear.

## The engine

One run (`Router.route`, off the main actor on a `RoutingProblem` snapshot):

1. **Problem** (`RoutingProblemBuilder`): the scope's nets, each with its
   copper groups (components), width, clearance, via, allowed and
   preferred layers, group; every other pad, trace, via, hole and the
   outline as obstacles tagged with their net, so a net may run over its
   own copper and never another's.
2. **Grid** (`RoutingGrid`): cells `grid` apart over the board, one plane
   per routable layer, and for each distinct net profile (half width,
   clearance, via radius, via drill) a trace map and a via map: free, one
   net's, or blocked. Stamped by exact distance to each copper shape
   (`CopperGeometry`), plus the copper-to-edge distance and the
   hole-to-hole distance for vias.
3. **Search** (`RoutingSearch`): A* over layer × cell, octilinear or
   orthogonal moves, via cost, bend cost, direction and preference
   penalties; a connection joins one copper group to the rest of its net
   (`RoutingTerminals`). Deterministic: ties break by node.
4. **Negotiation** (`Router`, `RoutingState`): connections routed in order;
   a failed one is searched again relaxed, paying for other nets' copper
   (present cost × history), and the nets it crosses are ripped up and
   re-queued, within a rip budget per connection and the strategy's
   rounds. The ledger un-stamps exactly what it stamped.
5. **Cleanup** (`RoutingCleanup`): each new trace as chains between
   anchors (ends, where other copper attaches, via centres); straightened
   into octilinear runs, 90° corners chamfered, via hops over short runs
   removed, stubs and collinear points tidied. Every change is checked
   exactly against other nets' copper and the edge, and never lengthens.
6. **Verification and repair** (`RoutingVerifier`, `RoutingRepair`): the
   result is checked with the DRC's own geometry and tolerance (clearance,
   edge, width, via size, hole to hole, every end joined); copper that
   fails goes back to its raw route, else is re-routed with a margin (3
   tries); a net that still fails is left unrouted with the reason.
   Parity tests hold the verifier to the DRC.
7. **Tuning and pairs** (`RoutingTuning`, `RoutingPair`, `PairGeometry`):
   as above.
8. **Apply** (`RoutingApply`, `RoutingSession`): the result goes into the
   document as one undo step, marked `auto`; Stop applies what is legal so
   far, Cancel nothing.

**Guarantees.** What is applied passes the DRC's copper kinds (clearance,
edge, trace width, via, hole to hole, short) — measured on the three
fixtures, see Results. Same board, options and seed → same copper. Stop
and a time limit leave a legal partial result.

**Limits.** Through vias only (no blind or buried). Groups take two-pin
nets only. No push-and-shove: kept copper never moves (a deliberate scope
decision). No impedance control (the pair's gap is the user's). The pair
breakout is a short leg (above). No copper pours.

### The Trace tool's assist

With the toolbar's wand on (`traceAssist`, off by default, remembered), a
Trace drag builds an `AssistRouter` at the press: the same problem with
every pad and trace as an obstacle, the drawn net (the net of the copper
the trace starts on, else none, which makes everything foreign) at its
class width and clearance, a 0.1 mm grid on the drawn layer, no vias.
Each live leg is an A* search (at most 200 000 nodes) from where the leg
starts to where the cursor lands, straightened and verified; the preview
and the release use the same leg, so what is added is what was shown.
Where there is no legal leg the preview is red and the release adds
nothing ("No legal route there."). Space locks the assisted leg. **V**
(with or without the assist) adds the trace so far, a via of the net's
class size where it ends — refused, with "No room for a via there.", when
the via wouldn't keep clear of other nets' copper or holes — and goes on
from the via on the next trace layer (never a Plane), all one undo step.

## Results

Debug build (the test target's configuration), `RoutingOptions.standard`
unless said, `DesignRules.customDefault`, the fixtures in
`schematical/schematicalTests/Fixtures/` (copies of
`planning/plan-38-data/fixtures/`).

| Stage (plan) | AFU (115) | LFU (188) | RegFile (24) |
|---|---|---|---|
| Search only, pitch 0.1 (38-E) | 99, 7.91 s | 70, 9.36 s | 15, 7.83 s |
| + negotiation, Balanced, pitch 0.127 (38-F) | 115, 26.1 s, 115 vias, 1614 mm | 140, 348 s | 24, 29.1 s |
| Fast (38-F) | 114, 34.8 s | 150, 292 s | 24, 51.8 s |
| Thorough (38-F) | 115, 27.6 s | 143, 1227 s, 217 vias, 2330 mm | 24, 29.5 s, 31 vias, 315 mm |
| + cleanup and repair, Balanced (38-H) | 115, 27.6 s, 1582 mm, 115 vias | 147, 484 s, 2528 mm, 246 vias | 24, 32.5 s, 314 mm, 31 vias |
| Landing tier, Balanced (38-M, after merging main) | 115, 28.0 s, 1582 mm, 115 vias | 147, 456 s, 2528 mm, 246 vias | 24, 29.8 s, 314 mm, 31 vias |

After apply, the DRC on each (38-H): AFU 16 silkscreen issues (the
fixture's own placement), nothing else; LFU 41 `unrouted` (= 188 − 147),
nothing router-caused; RegFile 8 silkscreen, nothing else. Cleanup took
AFU's vertices 928 → 834 and wire 1614 → 1582 mm.

A length group of AFU's first four two-pin nets (38-I): 15.37, 15.75,
30.61, 12.68 mm → 26.03, 15.75, 30.61, 12.68 mm, `residual` (AFU is dense;
only the first found room for bumps). A pair of AFU's first two two-pin
nets (38-J): `uncoupled("no legal breakout to a pad")`, all 115 routed,
nothing router-caused in the DRC. A pair of adjacent header pins in the
real app (38-K): coupled, 4.09 mm uncoupled, 0 vias.

In the real app (38-K, Debug): AFU Balanced, 115 / 115 in 35.1 s, 115
vias, 1582 mm.

## Code map

| File (`schematical/schematical/Routing/`) | What |
|---|---|
| `RoutingOptions.swift` | The options. |
| `RoutingProblem.swift`, `RoutingProblemBuilder.swift` | The snapshot the engine works on, and building it from the document. |
| `RoutingGrid.swift`, `RoutingTerminals.swift` | The grid and its maps; a connection's start and goal cells. |
| `RoutingSearch.swift` | A*. |
| `Router.swift`, `RoutingState.swift`, `RoutingResult.swift` | A run: order, negotiation, the ledger, the result. |
| `RoutingCleanup.swift`, `RoutingVerifier.swift`, `RoutingRepair.swift` | Cleanup, verification, repair. |
| `RoutingLength.swift`, `Meander.swift`, `RoutingTuning.swift` | Length and length matching. |
| `PairGeometry.swift`, `RoutingPair.swift` | Differential pairs. |
| `RoutingApply.swift`, `RoutingSession.swift` | Putting a result into the document; the background run. |
| `AutorouteModel.swift`, `AutorouteSheet.swift`, `RouteCommands.swift` | The sheet, its model and the Route menu. |
| `AssistRouter.swift` | The Trace tool's assist and the V key's via check. |

Elsewhere: `Canvas/NetClass.swift`, `Canvas/NetRules.swift`,
`Canvas/NetList.swift`, `Canvas/NetsSidebar.swift` (the rules and the
sidebar), `Canvas/PCBCanvasView.swift` (`PCBTool.route`, `RouteTool`,
`TraceDrawing`), `DesignRuleCheck/DesignRuleCheck.swift` (`layerRule`,
`lengthMatch`, `diffPair`).

Tests (`schematical/schematicalTests/`): `Routing*Tests`, `Router*Tests`,
`Meander`, `PairGeometry`, `Net*Tests`, `AutorouteModelTests`,
`RouteToolTests`, `AssistRouterTests`, `TraceToolTests`,
`DesignRuleGoldenTests`; `RouterFixtureTests` runs the three fixtures.
