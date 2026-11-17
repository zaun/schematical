# plan-50 data

Measurements the plan-50 sub-plans (A–E) rest on, taken on 2026-11-16. Nothing here is code the app runs.

| File | What it is |
|---|---|
| `reference_formulas.py` | The impedance and current formulas plan-50-E ports to Swift, in Python. `python reference_formulas.py` (needs `scikit-rf` for its self-check) prints the fixtures below. |
| `fd_solver.py` | A 2-D finite-difference Laplace solver (microstrip, stripline, odd mode) used as the ground truth the closed forms were checked against. |
| `probe_gerber_attributes.py` | Reads a Gerber with gerbonara 1.6.3; the cross-check for plan-50-D's attribute output. |
| `probe_kicad_via_tenting.py`, `probe_kicad_stackup_roundtrip.py` | KiCad 10.0.8 probes (recorded outputs in their docstrings) for plan-50-C and plan-50-D's KiCad output. |

## Formula accuracy (closed form vs the FD solver; the FD solver itself matched scikit-rf's `MLine` to within 2.2% on 5 cases)

| Case | Formula | Cases | Worst error vs FD |
|---|---|---|---|
| Microstrip Z0 | Hammerstad–Jensen with Wheeler thickness correction (= scikit-rf `MLine`, quasi-static) | 8 (w 0.15–3 mm, h 0.099–1.51 mm) | −7.2% (w/h 0.71), all others within ±1.9% |
| Microstrip Z0 | IPC-2141 `87/√(εr+1.41)·ln(5.98h/(0.8w+t))` — **rejected** | same 8 | −11.1% |
| Differential microstrip | `2·Z0·(1 − 0.48·e^(−0.96 s/h))` (IPC-2141), Z0 from the row above | 5 | −3.5% |
| Centred stripline Z0 | Cohn `30π/√εr · K(k')/K(k)`, `k = tanh(π·we/2b)`, `we = w + ½(t/π)(1 + ln(4πw/t))` | 6 | +5.9% (thick copper close to the planes), others within ±2.8% |
| Offset stripline Z0 | `2·Za·Zb/(Za+Zb)`, `Zx` = centred stripline with `b = 2hx + t` | 5 | +9.7% (hA 0.099, hB 0.55), others within ±6.3% |
| Differential stripline | `2·Zodd`, Cohn coupled odd mode `ko = tanh(π·we/2b)/tanh(π(w+s)/2b)`; offset: same harmonic combination | 4 centred, 3 offset | centred within ±2.4%, offset within ±3.2% |
| IPC-2141 stripline `60/√εr·ln(4b/(0.67π(0.8w+t)))` — **rejected** | | 7 | −11.6% |
| IPC-2141 differential stripline `1 − 0.347·e^(−2.9 s/b)` — **rejected** | | 3 | +17.5% |

## Fixtures (`reference_formulas.py` output; the Swift tests assert these to 0.01 Ω / 0.0001 A / 0.01 mil)

| Id | Case | Value |
|---|---|---|
| F1 | microstrip, w 0.35, h 0.2104, t 0.035 mm, εr 4.4 | 51.711 Ω |
| F2 | microstrip, w 3.0, h 1.51, t 0.035 mm, εr 4.5 | 47.958 Ω |
| F3 | microstrip, w 0.2, h 0.0994, t 0.035 mm, εr 4.1 | 46.455 Ω |
| F4 | differential microstrip, w 0.15, s 0.15, h 0.2104, t 0.035 mm, εr 4.4 | 115.783 Ω |
| F5 | centred stripline, w 0.15 mm, b 1.3 mm (hA = hB = 0.6424), t 0.0152 mm, εr 4.6 | 84.142 Ω |
| F6 | offset stripline, w 0.15, hA 0.0994, hB 0.55, t 0.0152 mm, εr 4.3 | 51.723 Ω |
| F7 | offset differential stripline, w 0.12, s 0.12, hA 0.0994, hB 0.55, t 0.0152 mm, εr 4.3 | 91.547 Ω |
| F8a | current, 10 mil wide, 0.035 mm copper, ΔT 10 °C, external | 0.8855 A |
| F8b | current, 10 mil wide, 0.0152 mm copper, ΔT 10 °C, internal | 0.2418 A |
| F9 | width for 1 A, 0.035 mm copper, ΔT 10 °C, external | 11.826 mil |
| F10 | width for 50 Ω, microstrip h 0.2104, t 0.035 mm, εr 4.4 | 14.650 mil |
| F11 | width for 50 Ω, microstrip h 1.51, t 0.035 mm, εr 4.5 | 110.12 mil (over the 100 mil limit) |
| F12 | standard 6-layer board, a trace on Layer 3 (Layers 2 and 4 planes: hA 0.55, hB 0.1164, t 0.0152 mm, εr = (4.6·0.55 + 4.16·0.1164)/0.6664 = 4.5231), w 0.15 mm | 53.659 Ω |
| F13 | the same layer, differential w 0.12, s 0.12 mm | 92.364 Ω |
| F14 | width for 50 Ω on the standard 6-layer Top (h 0.0994, εr 4.1, t 0.035) | 6.881 mil |
| F15 | width for 100 Ω differential, standard 4-layer Top, s 0.15 mm | 8.418 mil |
| F16 | width for 1 A on a 0.0152 mm internal layer, ΔT 10 °C | 70.841 mil |

## KiCad 10.0.8 facts (probes in this folder)

- A per-via `(tenting (front no) (back yes))` in a `(version 20240108)` board loads: front NOT_TENTED (2), back TENTED (1); a via with no `tenting` key is FROM_BOARD (0).
- KiCad's own save writes `(tenting (front yes) (back yes))` inside `(setup`, and per-via `(tenting (front no) (back yes))`.
- A `(setup (stackup …))` in a 20240108 board loads and saves back with the same `copper_finish` strings (`"HAL lead-free"`, `"HAL SnPb"`, `"ENIG"`, `"OSP"`, `"None"`) and mask `color` strings (`"Green"`, `"Black"`, `"Red"`, `"Blue"`, `"Yellow"`, `"White"`, `"Purple"`).
- KiCad's `BoardThickness` in a 4-layer `.gbrjob` is 1.6062 = copper 2·0.035 + 2·0.0152 + dielectrics 2·0.2104 + 1.065 + solder mask 2·0.01.

## Gerber spec facts (Revision 2026.05, read from the PDF text; section numbers are the PDF's)

- §3.4.3–3.4.4: `%`, `*`, `\` must be escaped in any string and `,` in any field, as `\u` + 4 hex digits (`,` is a comma).
- §5.3: aperture attributes in the dictionary at `%ADD` time are attached to that aperture, and **at `G36` to the region**; attributes cannot be changed after. §5.4: object attributes attach to every D01/D03/region created while they are in the dictionary; commands are not allowed inside a region. §5.5: `%TD*%` clears all aperture and object attributes, `%TD.N*%` one.
- §5.6.10 `.AperFunction` on copper: `ComponentPad`, `SMDPad,(CuDef|SMDef)`, `ViaPad`, `ConnectorPad`, `HeatsinkPad`, `Conductor`, `OtherPad,<text>`, `WasherPad`, `AntiPad` …; on drill files: `ViaDrill`, `ComponentDrill`, `MechanicalDrill`. "One function, one aperture", also one drill tool.
- §5.6.13 `.N,<net>`: on every copper object; `%TO.N,*%` is no net, `N/C` a single-pad net. `.P,<refdes>,<pin>` only on component pads; `.C,<refdes>`.
- §5.6.8 `.ProjectId,<Name>,<GUID>,<Revision>`, GUID RFC 4122 v1 or v4.
- The Job file (`.gbrjob`) schema (Ucamco `gerber-job-file-schema_en.json`, draft-07, "revision 2023.06"): top-level `Header`, `GeneralSpecs`, `MaterialStackup`, `DesignRules`, `FilesAttributes`; no key is required at top level; `GeneralSpecs` allows `ProjectId{Name,GUID,Revision}`, `Size{X,Y}`, `LayerNumber`, `BoardThickness`, `Finish`, `ImpedanceControlled`, …; `MaterialStackup[].Type` is a free string (KiCad writes `Legend`, `SolderPaste`, `SolderMask`, `Copper`, `Dielectric`).

## Fab stackups (JLCPCB, https://jlcpcb.com/impedance and /capabilities/pcb-capabilities, read 2026-11-16)

- 4-layer 1.6 mm (JLC04161H-7628): Top Cu 0.035, prepreg 7628 0.2104 (εr 4.4), L2 Cu 0.0152, core 1.065 (εr 4.6), L3 Cu 0.0152, prepreg 7628 0.2104 (εr 4.4), Bottom Cu 0.035 mm.
- 6-layer (JLC06161H-3313): Top 0.035, prepreg 3313 0.0994 (εr 4.1), L2 0.0152, core 0.55 (4.6), L3 0.0152, prepreg 2116 0.1164 (4.16), L4 0.0152, core 0.55 (4.6), L5 0.0152, prepreg 3313 0.0994 (4.1), Bottom 0.035 mm.
- 2-layer: the page states εr 4.5 and no layers. Finishes: HASL (leaded / lead-free), ENIG, OSP; HASL is not offered for 6 layers or more.
- PCBWay's stackup page (`/multi-layer-stackup.html`) returned 404, so no PCBWay numbers are used.
