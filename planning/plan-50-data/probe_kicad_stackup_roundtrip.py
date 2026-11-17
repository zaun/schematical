"""KiCad 10.0.8, same python as probe_kicad_via_tenting.py. Loads boards that have a `(setup (stackup ...))` and saves
them again, so the saved file shows what KiCad accepted. Inputs were a 2-layer exported board with this block
inserted at the start of `(setup`, for (color, finish) in Green/"HAL lead-free", Black/"HAL SnPb", Red/ENIG, Blue/OSP,
Yellow/None, White/ENIG, Purple/"HAL lead-free":
    (stackup (layer "F.SilkS" (type "Top Silk Screen")) (layer "F.Paste" (type "Top Solder Paste"))
      (layer "F.Mask" (type "Top Solder Mask") (color "<color>") (thickness 0.01))
      (layer "F.Cu" (type "copper") (thickness 0.035))
      (layer "dielectric 1" (type "core") (thickness 1.51) (material "FR4") (epsilon_r 4.5) (loss_tangent 0.02))
      (layer "B.Cu" (type "copper") (thickness 0.035))
      (layer "B.Mask" (type "Bottom Solder Mask") (color "<color>") (thickness 0.01))
      (layer "B.Paste" (type "Bottom Solder Paste")) (layer "B.SilkS" (type "Bottom Silk Screen"))
      (copper_finish "<finish>") (dielectric_constraints no))
Recorded: every saved file kept the same `(copper_finish ...)` and `(color ...)` strings, `(thickness 1.6)` general."""
import pcbnew, sys
for src, dst in zip(sys.argv[1::2], sys.argv[2::2]):
    pcbnew.SaveBoard(dst, pcbnew.LoadBoard(src))
