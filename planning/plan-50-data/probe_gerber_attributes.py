"""Reads a Gerber file with gerbonara 1.6.3 and prints its file attributes and, per object kind, the object
attributes (.N .P) and the aperture attributes (.AperFunction) it found. Run:
  python probe_gerber_attributes.py <file.gbr>      (gerbonara from a venv; this file is plan-50 data)
Recorded on KiCad 10.0.8's own AFU-F_Cu.gbr: file attrs .FileFunction=Copper,L1,Top, .ProjectId=AFU,<guid>,rev?;
first object is a Flash with attrs {.P: (R1, 1), .N: (/{slash}AFU,)} and aperture attrs
(.AperFunction, (SMDPad, CuDef))."""
import sys, collections
from gerbonara import GerberFile

f = GerberFile.open(sys.argv[1])
print("file attributes:")
for k, v in f.file_attrs.items():
    print("  ", k, v)
kinds = collections.Counter()
nets = set()
for o in f.objects:
    ap = tuple(getattr(getattr(o, "aperture", None), "attrs", ()) or ())
    func = next((v for k, v in ap if k == ".AperFunction"), None)
    kinds[(type(o).__name__, func)] += 1
    if ".N" in o.attrs:
        nets.add(o.attrs[".N"])
print("objects by (kind, AperFunction):")
for key, n in sorted(kinds.items(), key=lambda kv: str(kv[0])):
    print("  ", key, n)
print("distinct .N values:", len(nets))
