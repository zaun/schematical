"""KiCad 10.0.8 probe for plan-52-F: reads a .kicad_pcb and prints each zone's pad connection, thermal gap and spoke
width, min width, clearance, and each rule area's layers and what it blocks. Run with KiCad's bundled python:

  P=~/Applications/KiCad/KiCad.app/Contents/Frameworks/Python.framework/Versions/Current
  PYTHONPATH=$P/lib/python3.9/site-packages $P/bin/python3 -I probe_kicad_zones.py board.kicad_pcb

Recorded 2026-11-21 (the two boards in this folder, edited as the plan says):
  (connect_pads (clearance 0.3))          -> ZONE_CONNECTION_THERMAL
  (connect_pads yes (clearance 0.3))      -> ZONE_CONNECTION_FULL
  (thermal_gap 0.6) (thermal_bridge_width 0.4) (min_thickness 0.25) -> gap 0.6 spoke 0.4 minwidth 0.25, clearance 0.3
  zone_pouronly.kicad_pcb rule area: layers ['F.Cu', 'B.Cu'], pour blocked True, tracks/vias/pads/footprints False
  After ZONE_FILLER.Fill: the GND fill does not cover the keepout, and stays clear of the 0.25 mm trace by its clearance.
"""
import sys
import pcbnew

board = pcbnew.LoadBoard(sys.argv[1])
names = {getattr(pcbnew, n): n for n in dir(pcbnew) if n.startswith("ZONE_CONNECTION_")}
for zone in board.Zones():
    if zone.GetIsRuleArea():
        print("rule area layers", [board.GetLayerName(l) for l in zone.GetLayerSet().Seq()],
              "pour blocked", zone.GetDoNotAllowZoneFills(), "tracks", zone.GetDoNotAllowTracks(),
              "vias", zone.GetDoNotAllowVias(), "pads", zone.GetDoNotAllowPads(),
              "footprints", zone.GetDoNotAllowFootprints())
    else:
        print("zone", zone.GetNetname(), names.get(zone.GetPadConnection()),
              "gap", pcbnew.ToMM(zone.GetThermalReliefGap()), "spoke", pcbnew.ToMM(zone.GetThermalReliefSpokeWidth()),
              "minwidth", pcbnew.ToMM(zone.GetMinThickness()), "clearance", pcbnew.ToMM(zone.GetLocalClearance()))
