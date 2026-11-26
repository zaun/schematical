# plan-67-F: the KiCad check of a part drawn as its units

What was run, on 2026-11-25, with KiCad 10.0.8
(`~/Applications/KiCad/KiCad.app/Contents/MacOS/kicad-cli`, `KC` below).

1. The app's own sample `ExportSamples.split()` (a quad NAND SN74HC00N drawn as
   its units; unit B moved, flipped and turned to 90; `1Y` to an LED, a
   resistor to ground, and to unit B's `2A`; VCC and GND on rails) was exported
   by the test host, which prints each file as a `KICAD_FILE split/<name>
   <base64>` line:

       TEST_RUNNER_KICAD_OUT=1 xcodebuild -project schematical/schematical.xcodeproj \
           -scheme schematical -derivedDataPath /tmp/p67-dd CODE_SIGNING_ALLOWED=NO test \
           -only-testing:schematicalTests/KiCadExportTests > /tmp/p67/xb.log

2. The three files were decoded into `/tmp/p67-kicad/split/` and read by KiCad:

       KC sch export netlist -o /tmp/p67-kicad/split.net /tmp/p67-kicad/split/split.kicad_sch
       KC sch erc -o /tmp/p67-kicad/split.rpt /tmp/p67-kicad/split/split.kicad_sch

3. `python3 check_netlist.py /tmp/p67-kicad/split.net U1` → `split.netlist.txt`.

## Results

- **One component.** `components: 3; with ref U1: 1` (U1, D1, R1), value
  `SN74HC00N`, footprint `Schematical:PDIP-14`.
- **Five units**, KiCad's names A–E: pins 1 2 3 / 4 5 6 / 8 9 10 / 11 12 13 /
  7 14. The app's `U1-P` (VCC, GND) is KiCad's unit E.
- **The app's nets.** Pin 3 (`1Y`) is on one net with pin 4 (`2A`, on the
  turned and flipped unit B) and the LED; pin 14 on `5V`; pin 7 on `GND` with
  the resistor; the ten unwired pins each on their own `unconnected-(…)` net.
  All 14 pins are on a net.
- **ERC** (`split.erc.rpt`): 18 findings, none about units:
  `grep -cE 'missing_unit|extra_units|different_unit' split.erc.rpt` → 0. They
  are 10 `pin_not_connected` (the unwired pins), 7 `pin_not_driven` and 1
  `pin_to_pin`, the findings any such drawing has.

`check_netlist.py` needs only Python 3.
