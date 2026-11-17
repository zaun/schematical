"""KiCad 10.0.8 (its bundled python: Contents/Frameworks/Python.framework/Versions/Current/bin/python3 -I, with
PYTHONPATH=.../lib/python3.9/site-packages). Loads a board whose vias carry per-via tenting and prints each via's
front and back tenting mode. Input: a copy of an exported KiCad board (`(version 20240108)`, 2 layers) with two vias
appended before its final ")":
    (via (at 50 50) (size 0.8) (drill 0.4) (layers "F.Cu" "B.Cu") (tenting (front no) (back yes)) (uuid "..."))
    (via (at 52 50) (size 0.8) (drill 0.4) (layers "F.Cu" "B.Cu") (uuid "..."))
Recorded output:
    (50000000, 50000000) 2 1        front NOT_TENTED(2), back TENTED(1)
    (52000000, 50000000) 0 0        no tenting key: FROM_BOARD(0) both
    0 2 1                           TENTING_MODE_FROM_BOARD, NOT_TENTED, TENTED"""
import pcbnew, sys
b = pcbnew.LoadBoard(sys.argv[1])
for t in b.GetTracks():
    if t.Type() == pcbnew.PCB_VIA_T:
        print(t.GetPosition(), t.GetFrontTentingMode(), t.GetBackTentingMode())
print(pcbnew.TENTING_MODE_FROM_BOARD, pcbnew.TENTING_MODE_NOT_TENTED, pcbnew.TENTING_MODE_TENTED)
