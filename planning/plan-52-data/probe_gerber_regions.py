"""Reads a Gerber with gerbonara 1.6.3 and prints how many `Region` objects it has, by polarity, so plan-52-G's pour
output can be checked: one dark region per piece of fill, and clear-polarity regions for its holes.

  python probe_gerber_regions.py <file.gbr>      (gerbonara from a venv)

gerbonara 1.6.3 reads regions but does not expose their object attributes (`Region.attrs` is the dataclass `Field`
object, not a dict), so the pour's net is checked in the file text: `%TO.N,GND*%` then `%TA.AperFunction,Conductor*%` then
`G36*` (see the recorded output in plan-52-G).

Recorded 2026-11-21 on the app's own `pours.GTL` (ExportSamples.pours()): objects by kind {'Region': 29, 'Line': 2,
'Flash': 28}; region dark=True x1, dark=False x28 (the 28 clear rings of the pads of other nets and the keep-out);
`pours.GBL`: {'Region': 28, 'Flash': 28}, dark=True x1, dark=False x27.
"""
import collections
import sys
from gerbonara import GerberFile

f = GerberFile.open(sys.argv[1])
print("objects by kind:", dict(collections.Counter(type(o).__name__ for o in f.objects)))
polarity = collections.Counter(bool(o.polarity_dark) for o in f.objects if type(o).__name__ == "Region")
for dark, n in sorted(polarity.items()):
    print(f"  region dark={dark} x{n}")
