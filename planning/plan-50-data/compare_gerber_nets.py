"""Compares two Gerber copper files with gerbonara: the (reference, pin) -> net map of their pads, and how many
objects carry each .AperFunction. Prints each pad whose net differs between the files (empty = equal) and then the
two function counts.

    python compare_gerber_nets.py ours.GTL kicad-F_Cu.gbr

Recorded in plan-50-D on `ExportSamples.vias()` and `fourLayer()`: no differing pad."""
import collections
import sys

from gerbonara import GerberFile


def pads(path):
    result = {}
    counts = collections.Counter()
    for o in GerberFile.open(path).objects:
        aperture_attrs = dict(getattr(getattr(o, "aperture", None), "attrs", ()) or ())
        function = aperture_attrs.get(".AperFunction")
        counts[function[0] if function else None] += 1
        if ".P" in o.attrs:
            reference, pin = o.attrs[".P"][:2] if len(o.attrs[".P"]) > 1 else (o.attrs[".P"][0], "")
            result.setdefault((reference, pin), set()).add(tuple(o.attrs.get(".N", ())))
    return result, counts


ours, our_counts = pads(sys.argv[1])
theirs, their_counts = pads(sys.argv[2])
for key in sorted(set(ours) | set(theirs)):
    if ours.get(key) != theirs.get(key):
        print("DIFFERENT", key, "ours", ours.get(key), "kicad", theirs.get(key))
print("ours  ", dict(our_counts))
print("kicad ", dict(their_counts))
