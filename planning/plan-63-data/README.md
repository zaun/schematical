# plan-63 data

Measurements the plan-63 sub-plans (A–G, the schematic ERC) rest on, taken on 2026-11-25 with KiCad 10.0.8
(`~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`). Nothing here is code the app runs.

| File | What it is |
|---|---|
| `probe_pin_matrix.py` → `.txt` | One schematic with a 1-pin symbol per KiCad pin type and one two-pin net per pair; prints what `kicad-cli sch erc` reported per pair and per lone pin. |
| `probe_wires_labels.py` → `.txt` | One row per wire / label / No Connect case; prints the rules each row produced. |
| `probe_label_shapes.py` → `.txt` | Whether a global label's shape (input, output, …) changes any ERC result (it does not). |
| `probe_names.py` → `.txt` | Which rule fires for a label and a power symbol, two power symbols, two labels on one wire. |
| `probe_nets.py` → `.txt` | How many violations a three-pin net gives, and what a No Connect flag exempts. |
| `count_roles.py` → `.txt` | Pin roles per kind file in `schematical/schematical/Parts/*.json`, and the parts that name an output enable. |

Run any probe: `python3 -I planning/plan-63-data/probe_pin_matrix.py <outdir>`.

## KiCad's rules, and what each means for this app

The stock templates' `.kicad_pro` files (`SharedSupport/template/*/*.kicad_pro`) carry the rule names and default
severities; `kicad-cli sch erc --format json` lists the checks that are off by default (`ignored_checks`):
`single_global_label`, `four_way_junction`, `simulation_model_issue`, `footprint_filter`. The JSON also reports one
rule the templates do not list, `isolated_pin_label` (warning).

**Implemented (16)**, with the default severity and what the probes showed:

| Rule | Default | Meaning, as measured |
|---|---|---|
| `pin_to_pin` | by matrix | Two pins on one net whose pair the 12×12 pin map marks warning or error. The map is below. |
| `pin_not_connected` | error | A pin, not of type no_connect or free, that is alone on its net: unwired, or wired only to wires that go nowhere. A pin wired to a label is connected. A no_connect-typed pin is never a connection, so a pin wired only to one is not connected either. A pin marked with a No Connect flag is exempt. |
| `pin_not_driven` | error | A net with an input pin and no driver, when the net has no power_in pin. Drivers: output, bidirectional, tri_state, passive, power_out. Not drivers: input, free, unspecified, open_collector, open_emitter, no_connect. A lone input is reported. |
| `power_pin_not_driven` | error | A net with a power_in pin and no power_out pin. Only power_out drives a power_in pin (output, bidirectional, tri_state, passive do not). On a net with a power_in pin and an input pin, only this rule is reported. A lone power_in pin is reported. |
| `no_connect_connected` | warning | A No Connect flag on a pin whose net has another pin, or a pin of type no_connect that is wired. |
| `no_connect_dangling` | warning | A No Connect flag that is on no pin. |
| `wire_dangling` | error | A wire with neither end connected to anything. |
| `unconnected_wire_endpoint` | warning | Each wire end connected to nothing (a wire with both ends free reports this twice, and `wire_dangling` once). |
| `label_dangling` | error | A label whose wire-connected group (wires and labels, not merging labels of one name) has no pin. Two same-name labels on pinless groups are each dangling. |
| `isolated_pin_label` | warning | A label whose wire-connected group has exactly one pin and whose name no other label has (two labels of one name, even on groups with no pin, report nothing: measured on the fixtures' exports, plan-63-G). |
| `multiple_net_names` | warning | Two labels of different names, or two power symbols of different names, on one wire-connected group. A label and a power symbol of different names: nothing (`probe_names.txt`). |
| `similar_labels` | warning | Two labels whose names differ only in case. |
| `similar_label_and_power` | warning | A label whose name differs only in case from a power symbol's. |
| `duplicate_reference` | error | Two parts with one reference. (`kicad-cli` 10.0.8 reported nothing for two `U1` in the probe, so only the rule's name and severity are KiCad's; unit tests cover it.) |
| `single_global_label` | ignore | A label name used by one label only (20 of 20 unique names reported with the rule on). |
| `four_way_junction` | ignore | A junction with four wire ends (a T-junction is not reported). |

**Not applicable, and why** (29 of the 44 names in the templates; the other 15 are implemented, and `isolated_pin_label` is the 16th implemented rule):

| Rules | Why |
|---|---|
| `bus_definition_conflict`, `bus_entry_needed`, `bus_label_syntax`, `bus_to_bus_conflict`, `bus_to_net_conflict`, `net_not_bus_member` | KiCad buses carry members (`D[0..7]`) and need entries. A bus here is a set of named single wires with ports (`BusModel.swift`: `BusWire.name`, `BusEnd`); it has no member syntax and no entries, and a bus wire joins the labels of its own name by name (`NetlistBuilder.build`), not by members. |
| `different_unit_footprint`, `different_unit_net`, `extra_units`, `missing_bidi_pin`, `missing_input_pin`, `missing_power_pin`, `missing_unit`, `unit_value_mismatch` | Multi-unit symbols: a part here is one whole package. |
| `duplicate_sheet_names`, `hier_label_mismatch`, `global_label_dangling` | One sheet, no hierarchy, one kind of label (sheet-wide, matched by name). |
| `same_local_global_label` | KiCad warns when a local label has the name of a power symbol (`probe_names.txt`). Here a label named like a rail (`5V`, `GND`) joins that rail on purpose (`NetlistBuilder.build`, documented in `docs/file-format.md`), so the rule would flag a supported feature. |
| `conflicting_netclasses` | A net class is assigned to a net on the PCB page, not by the drawing. |
| `endpoint_off_grid` | Every point snaps to the 10-unit grid (`SchematicGrid.snap`; `SchematicFile.label` snaps on open). |
| `footprint_filter`, `footprint_link_issues`, `lib_symbol_issues`, `lib_symbol_mismatch`, `simulation_model_issue`, `unresolved_variable` | No symbol library, no footprint filters, no SPICE models, no text variables. |
| `label_multiple_wires` | Measured: two wires ending on one label point reported nothing (`probe_wires_labels.txt`). |
| `similar_power` | A rail's name is generated from its voltage (`SupplyChoice.name(volts:)`: `3V3`, `170V`); two rails differing only in case cannot be made. |
| `unannotated` | A part always has a reference when placed (`takeReference`), and Rename refuses a duplicate (`renameChip`). |

## KiCad's pin types, in the order of the pin map

`input, output, bidirectional, tri_state, passive, free, unspecified, power_in, power_out, open_collector,
open_emitter, no_connect` (indices 0–11). Default pin map (0 ok, 1 warning, 2 error), from the stock templates and
confirmed by `probe_pin_matrix.txt` for every cell the probe could see (the no_connect row and column are reported
through `pin_not_connected` and `no_connect_connected`, not `pin_to_pin`):

```
              in out bi tri pas fre uns pwI pwO oc  oe  nc
input          0  0  0  0   0   0   1   0   0   0   0   2
output         0  2  0  1   0   0   1   0   2   2   2   2
bidirectional  0  0  0  0   0   0   1   0   1   0   1   2
tri_state      0  1  0  0   0   0   1   1   2   1   1   2
passive        0  0  0  0   0   0   1   0   0   0   0   2
free           0  0  0  0   0   0   0   0   0   0   0   2
unspecified    1  1  1  1   1   0   1   1   1   1   1   2
power_in       0  0  0  1   0   0   1   0   0   0   0   2
power_out      0  2  1  2   0   0   1   0   2   2   2   2
open_collector 0  2  0  1   0   0   1   0   2   0   0   2
open_emitter   0  2  1  1   0   0   1   0   2   0   0   2
no_connect     2  2  2  2   2   2   2   2   2   2   2   2
```

A global label's shape (input, output, bidirectional, tri_state, passive) changed nothing in any of 20 cases
(`probe_label_shapes.txt`), so a label's type here never feeds the ERC.

## Roles to KiCad types (the mapping plan-63-B implements)

Measured by `count_roles.py` (`count_roles.txt`): pin entries over every pinout of every kind file, by role:
`inout` 63158, `input` 21756, `output` 12915, `power` 7237, `ground` 7108, `nc` 3895, `absent` 86. Of the 40174
parts, 3756 name an output enable; they are in 11 kinds (buffer, converter, counter, dff, eeprom, latch, mux,
register, serial, shift, sram).

The kinds whose every pin KiCad types `passive` (its stock symbols for them do): connector, header, socket,
resistor, capacitor, inductor, diode, led, fuse, crystal, relay, buzzer, switch, transistor, 7seg, nixie,
dekatron, lcd. Their `inout`, `input` and `output` pins are `passive`; their `nc` pins stay `no_connect`.

## How KiCad counts (`probe_nets.txt`)

- `pin_to_pin`: three outputs on one net give **two** violations, two outputs and an input give one, three
  `power_out` give two. Rule used here: walking the net's pins in order, each pin that conflicts with an earlier one
  is reported once, against the first earlier pin it conflicts with.
- `pin_not_driven` and `power_pin_not_driven`: **one violation per net** (three inputs: one; two `power_in` and an
  input: one, the power one), not one per pin.
- A No Connect flag on a pin, or at the far end of a wire from it, exempts the pin from `pin_not_connected`,
  `pin_not_driven` and `power_pin_not_driven`: an input, a `power_in` and an output each with a flag on them report
  nothing. With a flag on a pin that shares its net with another pin the only report is `no_connect_connected`; in
  particular the other input of "two inputs wired together, flag on one" is not reported as not driven.

## KiCad's message for each rule (the `description` of its JSON violation)

| Rule | Message |
|---|---|
| `pin_to_pin` | `Pins of type <A> and <B> are connected` (types as `Output`, `Power output`, `Open collector`, `Bidirectional`, `Tri-state`, `Passive`, `Input`, `Power input`, `Unspecified`, `Free`, `Open emitter`, `Unconnected`) |
| `pin_not_connected` | `Pin not connected` |
| `pin_not_driven` | `Input pin not driven by any Output pins` |
| `power_pin_not_driven` | `Input Power pin not driven by any Output Power pins` |
| `no_connect_connected` | `A pin with a "no connection" flag is connected`; for a no_connect-typed pin `Pin with 'no connection' type is connected` |
| `no_connect_dangling` | `Unconnected "no connection" flag` |
| `wire_dangling` | `Wires not connected to anything` |
| `unconnected_wire_endpoint` | `Unconnected wire endpoint` |
| `label_dangling` | `Label not connected` |
| `isolated_pin_label` | `Label connected to only one pin` |
| `multiple_net_names` | `Both <A> and <B> are attached to the same items; <A> will be used in the netlist` |
| `similar_labels` | `Labels are similar (lower/upper case difference only)` |
| `similar_label_and_power` | `Power pin and label are similar (lower/upper case difference only)` |
| `single_global_label` | `Global label only appears once in the schematic` |
| `four_way_junction` | `Four items connected at <x>, <y>` |
| `duplicate_reference` | not reported by `kicad-cli` 10.0.8 in the probe; this app's text is `Duplicate reference <R>` |
