# Routing

The PCB page routes copper for you: **Route ▸ Autoroute…** routes the whole
board (or a selection) with the options you pick, the **Route** tool routes
one net or one unrouted line with a click, and the **Trace** tool's
**Assist** makes a hand-drawn trace walk around other nets' copper. All of
it obeys the board's design rules: what the router puts down passes the
Design Rule Check (⇧⌘D) for the kinds it is responsible for, or it isn't
put down. The rules it follows per net come from **net classes**, **layer
rules** and **net groups**, set in Board Details' Nets, Net Classes and
Net Groups tabs (PCB sidebar, nothing selected ▸ Board Details…) and saved in the file
([file-format.md § Net classes](file-format.md#net-classes),
[§ Net layer rules](file-format.md#net-layer-rules),
[§ Net groups](file-format.md#net-groups)).

Plan: `planning/completed/plan-38-*.md`; the prototype it was measured
against and the three fixture boards: `planning/plan-38-data/`. The
planning, improvement and straight-pad-exit stages, the runs Balanced and
Thorough choose between, and the Default class from the design rules came
later (2026-11); what they measure is in [Results](#results). Push and
shove, an option that moves other nets' unlocked copper aside instead of
ripping up, is `planning/completed/plan-66-*.md`.

## Commands

| Command | Key | What it does |
|---|---|---|
| Route ▸ Autoroute… | ⌥⌘R | The Autoroute sheet: options, then Route. |
| Route ▸ Route Selected Nets | ⌥⇧⌘R | Routes the selection's nets with the last options, no sheet. |
| Route ▸ Unroute Selected | ⌥⌘U | Removes the selection's nets' unlocked traces and vias. |
| Route ▸ Unroute All Autorouted | — | Removes every unlocked trace and via the router put down (`auto`). |
| Route ▸ Select Autorouted Copper | — | Selects every trace and via the router put down. |
| Route tool (PCB toolbar) | — | Click a pad: route its net. Click an unrouted (ratsnest) line: route that one connection. Uses the last options. |
| Nets row ▸ Route Net / Unroute Net | — | The same for one net, from its row's context menu in Board Details ▸ Nets (Route Net closes Board Details first). |
| Trace tool Assist (wand) | — | A dragged trace walks around obstacles; red where no legal way exists. |
| V while drawing a trace | V | Drops a via where the trace has got to and goes on on the next trace layer. |

The Route menu and the toolbar's Autoroute button are on the PCB page only,
and not while the simulation runs.

### The Autoroute sheet

The sheet shows how many nets and connections the options would route,
then while running a progress bar ("83 of 115 connections routed · GND")
with **Stop** and **Cancel**, and the board behind it draws the copper so
far at 60 %. The line reads "Planning, pass 3" while the plan is made and
"Improving" while routed connections are taken up again; both are part
of Balanced and Thorough, and on a large board most of the time goes to
improving. **Stop** keeps what is routed (a legal partial result);
**Cancel** puts nothing down. When it finishes it lists each net still
unrouted with how many connections and why ("no legal way found", "run
stopped first", an impossible net's reason, a pair's or group's outcome),
and a **Design Rule Check** button. A run that moved existing copper (Push
and shove) says so under the first line ("Moved 3 traces and 1 via aside").
Clicking a listed net highlights it.
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
| Strategy (`strategy`) | Fast; Balanced; Thorough | Balanced | How hard the run tries (`Router.Strategy.of`). **Fast**: one run, the first legal way for each connection, 6 negotiation rounds, rip-up limit 8 per connection; a quick draft with the most vias. **Balanced**: 4 runs side by side, each planned (14 passes), negotiated (30 rounds, limit 12) and improved (up to 20 passes); the best is kept. **Thorough**: Balanced's 4 runs and 4 more at other via prices, 8 side by side; the best of a superset, so never worse than Balanced, and slower (LFU 292 s against 165 s on 12 cores). |
| Grid (`grid`) | 0.05, 0.1, 0.127, 0.254 mm | 0.127 mm (5 mil) | The search grid; 0.127 divides the 0.635 mm component grid. Pad centres are mostly *not* on it all the same (a pad's centre is where its footprint puts it: 121 of AFU's 168, 245 of LFU's 247 and all 32 of RegFile's are off), so a trace's last leg to a pad is made straight afterwards (cleanup, below). |
| Width (`widthOverride`) | mil, or off | off | Every net this width instead of its class's. |
| Clearance (`clearanceOverride`) | mm, or off | off | Every net this clearance instead of its class's (never under the DRC's). |
| Fan out to Planes (`fanoutPlanes`) | on/off | on | A pad of a net that a Plane layer carries gets a short trace and a via to the plane instead of a route. A pad a copper pour joins (its net, touched by the pour's fill) is already connected and needs no route either. |
| Tune length groups (`tuneGroups`) | on/off | on | Length-match groups are tuned after routing. |
| Route pairs side by side (`routePairs`) | on/off | on | Differential pairs are routed coupled. |
| Push and shove (`pushAndShove`) | on/off | off | When a connection has no legal way, move the unlocked traces and vias of other nets aside, hand-drawn or not (locked copper never moves), instead of ripping up and giving up ([Push](#the-engine), step 6). Off by default because it can move copper you drew. |
| — (`seed`) | integer, not in the sheet | 1 | Kept in saved options; it no longer changes the result (the runs of Balanced and Thorough differ by what a via costs the first routing, not by seed). Same board and options, same copper. |
| Time limit (`timeLimit`) | seconds (60 when switched on), or off | off | Stop on its own after this long (as Stop): what is routed and improved so far is kept. |

## Rules per net

- **Net classes.** A net's class gives its trace width (mil), clearance,
  via drill and diameter, and optionally a layer rule. A net is in the
  first class any of whose members names it; others are in the Default
  class, which is the smallest copper the design rules in force allow
  (`NetRules.defaults(for:)`): the narrowest whole mil at or over the min
  trace width (never under 5 mil), the min clearance, and a via of the min
  via drill with the min via diameter, or the drill plus the min annular
  ring each side when that is wider. Under PCBWay's rules that is 5 mil
  (0.127 mm), 0.1 mm clearance and a 0.2 / 0.5 mm via; under another
  manufacturer's, that manufacturer's, so the Default class is always
  legal for the board's own design rules (saved in its file, changed in
  step 1 of the Design Rule Check). Small copper is
  what a low-voltage logic board wants and it leaves the router the most
  room (the fixtures route with fewer vias and more connections at it,
  see Results); nets that need more go in a class. The router
  routes each net at its class's width and keeps its class's clearance
  from every other net's copper (the larger of the two nets' clearances).
  The Trace tool and the Via tool use the same class values for what you
  draw, and a class you add starts as the Default class. A new schematic
  starts with five classes: Power 5V, 3V3, 12V and 24V (20 mil, a 0.4 / 0.8 mm
  via, each holding that supply rail) and Logic (the Default class's copper, no
  nets: signal nets have no voltage of their own, so you put them in); a file opened keeps only its own. Net Classes ▸
  Add Power Class makes a 20 mil class with a 0.4 / 0.8 mm via holding the
  supply and ground nets. Copper already on the board keeps the sizes it
  was made with, whatever the rules become.
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

`Router.route` works off the main actor on a `RoutingProblem` snapshot.
Fast makes one run; Balanced and Thorough make several at once, each on
its own thread, that differ only in what a via costs the first routing
(12 to 60 cells; Thorough adds four more, up to 80), and keep the result that routes the most
connections and then scores lowest (wire, plus each via as 80 cells of
wire and each corner as 1): how the first routing prices a via decides
which connections change layers, and no one price is best on every board.
One run:

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
   hole-to-hole distance for vias. The board's edge is its outline and every
   cutout (`BoardEdge`, plan-65): a cell inside a cutout is blocked on every
   layer and a cell within the copper-to-edge distance of a cutout's edge is
   blocked as it is at the outline; the verifier and the cleanup hold to the
   same edge, and a shortcut that crosses a cutout is never taken.
3. **Search** (`RoutingSearch`): A* over layer × cell, octilinear or
   orthogonal moves, via cost, bend cost, direction and preference
   penalties; a connection joins one copper group to the rest of its net
   (`RoutingTerminals`). Deterministic: ties break by node.
4. **Plan** (`RoutingPlan`; not Fast): before any copper goes down, every
   connection is searched over the fixed copper alone against a soft map
   of what the other nets' plans need (`RoutingDemand`: for each node, how
   many other nets' planned copper a trace centre there would be too near,
   by the same distances the grid stamps with). Each pass the nets whose
   plans share a place are planned again, sharing costs more, and the
   contested nodes are remembered (negotiated congestion). The plan that
   shared the least is kept. It stamps nothing: it only adds a price to
   the routing that follows, at the places other nets' plans still want,
   so the first connections routed leave the later ones their room.
5. **Negotiation** (`Router`, `RoutingState`): connections routed in order;
   a failed one is searched again relaxed, paying for other nets' copper
   (present cost × history), and the nets it crosses are ripped up and
   re-queued, within a rip budget per connection and the strategy's
   rounds. The ledger un-stamps exactly what it stamped.
6. **Push** (`RoutingPush`, `MovableChains`; with the option `pushAndShove`, in
   every strategy): a connection that has no legal way gets a way through
   copper that can move before anything is ripped up. A relaxed search
   finds a path through other nets' copper at a push price, and that copper
   is taken up, the connection laid (as far from where the copper was as
   the room allows, so there is room beside it) and each piece laid
   again at once inside a corridor `pushReach` (1.0 mm) round where it
   was, priced to stay near its old line, with the same ends, the same
   layers in the same order and no more vias than it had. A piece that
   needs room may push one more level (`pushDepth` 2), at most 12 pieces
   (`pushMaxVictims`), each search at most 200 000 nodes; nothing laid by
   the push is pushed again by it. All or nothing: if any piece finds no
   place the state goes back exactly as it was (a value copy, as the
   improvement's moves), and negotiation does what it does without the
   option. What can be pushed: the run's own connections (a connection
   that ends on another one of its net, a branch, is ripped instead) and
   the board's existing copper of nets the run does not route, cut into
   **chains** (`RoutingProblem.movable`): the stretch of trace and via
   from one anchor to the next, an anchor being a pad, a junction, a free
   end, the meeting of two traces or a via with other than two ends. A
   chain stays an ordinary obstacle when any piece of it is locked, it has
   two widths or via sizes, it is on a layer the run leaves out, other
   copper touches it away from its anchors, it has no two anchors, or it
   differs in width or via size from the net's first chain (a net has one
   routing profile). Chains are entries of the ledger under their own
   ("bystander") nets: stamped like routed copper, crossed only at a push
   price, never ripped up, and left exactly as they are by the cleanup
   unless a push touched them; a rule the user's own untouched copper
   already breaks is not repaired. A pushed chain is cleaned like new
   copper, checked, and either kept moved or put back as it was. Coupled
   pairs and Plane fan-outs are never pushed.
7. **Improvement** (`RoutingPolish`; not Fast): once everything that can
   be routed is, each connection is taken up again and routed afresh with
   a via priced at 80 cells and a corner at 1 — on its own; if it has a
   via, also through other nets' copper, which is then routed again too;
   and each net whole, as a new tree. A move is kept only when it leaves
   no more connections unrouted and the copper it touched scores lower;
   otherwise the state goes back exactly as it was, so the result never
   gets worse and is legal at every moment (Stop works here too). A move
   that was not kept is tried again only after copper within 4 mm of it
   has changed. Passes repeat until one keeps nothing. A net still
   unrouted is tried again each pass, in the room the moves have made. A
   coupled pair's copper is never taken up.
8. **Cleanup** (`RoutingCleanup`): each new trace as chains between
   anchors (ends, where other copper attaches, via centres); straightened
   into octilinear runs; each end made to leave its pad at 0°, 45° or 90°
   (the leg from an off-grid pad centre to the first grid point runs at
   whatever angle that takes: the leg and up to two vertices after it,
   never past an anchor, give way to the legal one- or two-segment route
   that adds the least length, at most 0.5 mm); 90° corners chamfered, via
   hops over short runs removed, stubs and collinear points tidied. Every
   change is checked exactly against other nets' copper and the edge, and
   but for those end legs none lengthens a trace. A coupled pair's copper
   is frozen: no pass changes it.
9. **Verification and repair** (`RoutingVerifier`, `RoutingRepair`): the
   result is checked with the DRC's own geometry and tolerance (clearance,
   edge, width, via size, hole to hole, every end joined); copper that
   fails goes back to its raw route, else is re-routed with a margin (3
   tries); a net that still fails is left unrouted with the reason.
   Parity tests hold the verifier to the DRC.
10. **Tuning and pairs** (`RoutingTuning`, `RoutingPair`, `PairGeometry`):
   as above. Pairs are routed first and kept through every later stage;
   length groups are tuned last, on the improved and cleaned copper of
   each run.
11. **Apply** (`RoutingApply`, `RoutingSession`): the result goes into the
   document as one undo step, marked `auto`; Stop applies what is legal so
   far, Cancel nothing. A moved chain's new points replace its vertices in
   the trace they came from (same id, width, layer and flags; its ends stay
   where they were, so its contacts do) and a moved via's centre is set;
   a chain the board has changed since the problem was made is skipped.

**Guarantees.** What is applied passes the DRC's copper kinds (clearance,
edge, trace width, via, hole to hole, short) — measured on the three
fixtures, see Results. Same board and options → same copper: each run is
deterministic and the choice between runs depends only on their results,
not on which thread finishes first. Stop and a time limit leave a legal
partial result (the best of the runs as they stood), which then depends
on when it was stopped.

**Limits.** Through vias only (no blind or buried). Groups take two-pin
nets only. Push and shove lays copper again on the grid rather than
nudging it: a hand-drawn trace that is pushed comes out cleaned
(straightened, chamfered) within `pushReach` of its old line, with its ends
where they were; the run's own nets' existing copper stays where it is
(its terminals come from it), so only other nets' copper and the run's new
connections move; a piece never gains a via or changes layer; locked
copper, copper something else touches away from its ends and copper on a
disabled layer never move; a branch is ripped, not pushed; the Trace
tool's Assist does not push. No teardrops (a board option, not planned).
No impedance control (the pair's gap is the user's). The pair
breakout is a short leg (above). No copper pours. The improvement moves
one connection or one net at a time, so vias that only a rearrangement of
several nets at once would remove stay; and it never moves a part: on a
board of surface-mount parts on one side, every connection that has to
cross another needs two vias, whatever the router.

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

### Now (2026-11): Balanced, the Default class from the design rules

An optimised build (`-configuration Release`; the improvement stage is
some 50 times slower unoptimised), a 12-core Mac, `RoutingOptions.standard`,
`DesignRules.customDefault` (PCBWay), the fixtures in
`schematical/schematicalTests/Fixtures/`. All three boards have
surface-mount pads only (AFU 168, LFU 247, RegFile 32; no through-hole
pad), on two layers. Printed by `RouterQualityFixtureTests`, which also
holds bounds a little over these.

| Board | Routed | Vias | Corners | Off 45°/90° | Wire | Time |
|---|---|---|---|---|---|---|
| AFU | 115 / 115 | 73 | 434 | 0 | 1429 mm | 12 s |
| RegFile | 24 / 24 | 16 | 99 | 0 | 292 mm | 2 s |
| LFU | 188 / 188 | 254 | 1010 | 1 | 3178 mm | 168 s |

Against the router as plan-38 left it (one run, no plan, no improvement,
no straight pad exits), measured the same day in the same build:

| Board | plan-38 router, old Default class (10 mil, 0.4 / 0.8 via) | plan-38 router, new Default class (5 mil, 0.2 / 0.5 via) | Now |
|---|---|---|---|
| AFU | 115 / 115, 115 vias, 372 corners (118 off-angle), 1582 mm | 115 / 115, 141 vias, 350 (113), 1456 mm | 115 / 115, 73 vias, 434 (0), 1429 mm |
| RegFile | 24 / 24, 31 vias, 96 (25), 314 mm | 24 / 24, 26 vias, 67 (26), 310 mm | 24 / 24, 16 vias, 99 (0), 292 mm |
| LFU | 147 / 188, 246 vias, 776 (166), 2528 mm | 177 / 188, 386 vias, 981 (176), 3181 mm | 188 / 188, 254 vias, 1010 (1), 3178 mm |

Since 2026-11-24 LFU's row differs (185 / 188, 289 vias, 1083 corners, 3247 mm): commit `f972c269` changed how
the netlist numbers nets (a bus wire named like a supply is on that rail), which reorders the router's ties.
Measured in the same Release build at its parent (188 / 188, 254 vias, 3178 mm) and at the commit (185 / 188,
289 vias, 3247 mm); the router's code was not touched. AFU and RegFile did not move.

What each stage adds, one run at via cost 12 unless said (AFU / RegFile):
the smaller Default class alone 141 / 26 vias; + improvement 91 / 18;
+ plan 73 / 16; the best of the four runs 73 / 16 (the single runs range
72 to 78 and 15 to 17). On LFU the single runs complete 181 to 188 of 188;
the run kept is the one that completes all of them. Straight pad exits
took AFU's off-angle corners from over 100 to 0 or 1 for 0.7 % more wire
(1417 → 1427 mm in the run it was measured on).

What is not good yet, and what was not measured:

- **Corners went up**, not down (AFU 372 → 434): each straightened pad
  exit adds one, and the improvement trades corners for vias. 77 of AFU's
  pieces are short jogs (under 0.6 mm between a turn one way and a turn
  the other); nothing removes those yet.
- **LFU takes 168 s**, most of it improving; Stop keeps the result so far.
- An unoptimised (Debug) build is far slower: AFU's Balanced run takes
  minutes there. Use Fast, or an optimised build.
- The via counts are what this router reaches, not a proven floor: no
  other router was run on these boards for comparison, and no board with
  through-hole parts was measured (the fixtures have none).
- Thorough gained little here: AFU the same result as Balanced in 20 s,
  RegFile 14 vias (303 mm) in 2.4 s, LFU the same result in 292 s.
  Fast, for comparison: AFU 115 / 115, 141 vias, 0.3 s; RegFile 24 / 24,
  29 vias, 3.8 s; LFU 168 / 188, 414 vias, 15 s.

### Push and shove (plan-66, 2026-11-25)

The same build and options as above (Release, `RoutingOptions.standard`, Balanced, a 12-core Mac), with
`pushAndShove` off and on, printed by `RouterQualityFixtureTests` (`testPushAFU`, `testPushRegFile`,
`testPushLFU` beside the off tests). The fixtures hold no copper, so what a push can move is what the run laid
itself; "pushes" are `RoutingStats.pushTried` and `pushKept`.

| Board | Option | Routed | Vias | Corners | Off 45°/90° | Wire | Time | Pushes tried / kept |
|---|---|---|---|---|---|---|---|---|
| AFU | off | 115 / 115 | 73 | 434 | 0 | 1429 mm | 11.6 s | 0 / 0 |
| AFU | on | 115 / 115 | 73 | 434 | 0 | 1429 mm | 10.9 s | 93 / 0 |
| RegFile | off | 24 / 24 | 16 | 99 | 0 | 292 mm | 2.1 s | 0 / 0 |
| RegFile | on | 24 / 24 | 16 | 99 | 0 | 292 mm | 2.0 s | 29 / 0 |
| LFU | off | 185 / 188 | 289 | 1083 | 0 | 3247 mm | 248 s | 0 / 0 |
| LFU | on | 187 / 188 | 213 | 916 | 1 | 3152 mm | 213 s | 10210 / 40 |

On AFU and RegFile every first search found a way or no push fitted, so the result is the off result,
copper for copper. On LFU, where negotiation rips up most, 40 of 10 210 pushes were kept and the run routes 2
more connections with 76 fewer vias and 95 mm less wire, in less time. The off rows are not the "Now" table's
LFU (188 / 188, 254 vias, 168 s): main at the commit before plan-66, built and run the same way, gives the
off row above exactly (185 / 188, 289 vias, 1083 corners, 3247 mm), so the difference came with other changes
to the router since "Now" was measured. Thorough off, the same day: AFU 115 / 115, 73 vias, 20 s; RegFile
24 / 24, 14 vias, 303 mm, 2.3 s; LFU 186 / 188, 280 vias, 3221 mm, 365 s.

On a board that already has copper (RegFile routed, then each of its 8 nets routed again at 12 to 18 mil with
the other nets' copper in the way, `RoutingPushIntegrationTests`): 8 of the 32 cases route all 3 connections
by moving the other nets' existing traces, where the same run without the option leaves 1 or 2 unrouted
(`LATCH1.13` at 14 mil: 2 / 3 off, 3 / 3 on, 2 traces and a via moved; `LATCH1.19` at 18 mil: 1 / 3 off, 3 / 3 on,
6 traces and 3 vias); none routes fewer than off, and every case is DRC-clean and undone exactly by one Undo.
At 20 mil and wider the card has no room for the net with or without the option.

### plan-38 (as landed), for the record

Debug build (the test target's configuration), `RoutingOptions.standard`
unless said, `DesignRules.customDefault`, the old Default class (10 mil,
0.4 / 0.8 mm via).

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
| `Router.swift`, `RoutingState.swift`, `RoutingResult.swift` | The runs and the choice between them; a run: order, negotiation, the ledger, the result. |
| `RoutingPlan.swift` | The plan before routing: the soft demand map and the negotiated passes. |
| `RoutingPolish.swift` | The improvement passes after routing. |
| `RoutingPush.swift`, `RoutingCorridor.swift`, `MovableChains.swift` | Push and shove: the transaction, the corridor and avoid fields it searches under, and cutting the board's existing copper into chains. |
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
`DesignRuleGoldenTests`; push and shove: `RoutingAnchorTests` (the output with
the option off, pinned), `RoutingChainTests`, `RoutingChainEngineTests`,
`RoutingPushTests`, `RoutingPushIntegrationTests`; `RouterFixtureTests` runs the three fixtures
(with Fast in an unoptimised build, `RoutingOptions.fixture`), and
`RouterQualityFixtureTests` Balanced and Thorough on them, in an
optimised build only:

    xcodebuild -project schematical/schematical.xcodeproj -scheme schematical \
        -configuration Release ENABLE_TESTABILITY=YES CODE_SIGNING_ALLOWED=NO \
        test -only-testing:schematicalTests/RouterQualityFixtureTests
