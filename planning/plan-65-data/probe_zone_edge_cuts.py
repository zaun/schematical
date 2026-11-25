"""KiCad 10.0.8: a zone drawn over the whole outline keeps out of an inner Edge.Cuts loop once filled.
Run with KiCad's bundled python:  PYTHONPATH=<KiCad.app>/Contents/Frameworks/Python.framework/Versions/Current/lib/python3.9/site-packages <that python3> -I probe_zone_edge_cuts.py"""
import pcbnew
board = pcbnew.LoadBoard("zone_edge_cuts.kicad_pcb")
pcbnew.ZONE_FILLER(board).Fill(board.Zones())
zone = board.Zones()[0]
mm = pcbnew.FromMM
inside_cutout = pcbnew.VECTOR2I(mm(20), mm(15))
beside_cutout = pcbnew.VECTOR2I(mm(10), mm(15))
near_edge = pcbnew.VECTOR2I(mm(14.9), mm(15))  # 0.1 mm from the cutout edge: inside the 0.5 clearance? (zone-to-edge)
print("B.Cu fill at the cutout centre:", zone.HitTestFilledArea(pcbnew.B_Cu, inside_cutout))
print("B.Cu fill 5 mm left of the cutout:", zone.HitTestFilledArea(pcbnew.B_Cu, beside_cutout))
print("B.Cu fill 0.1 mm left of the cutout edge:", zone.HitTestFilledArea(pcbnew.B_Cu, near_edge))
