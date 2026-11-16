"""Write planning/plan-43-data/add/usb.json (plan-43-C phase 3) from the numbers read off the two GCT drawings."""
import json
X = [(1, 3.2, 0.6), (2, 2.4, 0.6), (10, 1.75, 0.3), (3, 1.25, 0.3), (9, 0.75, 0.3), (4, 0.25, 0.3),
     (5, -0.25, 0.3), (8, -0.75, 0.3), (6, -1.25, 0.3), (7, -1.75, 0.3), (11, -2.4, 0.6), (12, -3.2, 0.6)]
def pads(y, h): return [{"pin": p, "x": x, "y": y, "width": w, "height": h, "shape": "rect"} for p, x, w in X]
def slot(x, y, cu, sl): return {"pin": 13, "x": x, "y": y, "width": 1.0, "height": cu, "shape": "round", "drill": 0.6, "slotLength": sl}
def hole(x, y): return {"pin": 0, "x": x, "y": y, "shape": "round", "drill": 0.65, "plated": False, "width": 0.65, "height": 0.65}
MAP = "Pins 1-12 = A1/B12, A4/B9, A5, A6, A7, A8, B5, B6, B7, B8, A9/B4, A12/B1; 13 = shell."
fp4105 = {"id": "USB-GCT-USB4105", "description": "GCT USB4105 USB-C receptacle, right angle, 12 SMD pads + 4 slotted shell legs",
  "note": "GCT drawing USB4105 rev B4 sheet 1 Recommended PCB Layout (as viewed from component side, tolerance 0.05), datasheets/connector-usb/usb4105.pdf (https://gct.co/files/drawings/usb4105.pdf; the product specification usb4105-spec.pdf prints no layout): pad centres 6.40/4.80/3.50 apart and 0.50 pitch inside, widths 0.60 (4x, A1/B12, A4/B9, A9/B4, A12/B1) and 0.30 (8x), length 1.15; front slots 0.60 x 1.40 (copper 1.00 x 1.80) 2.60 behind the PCB edge, 8.64 apart; rear slots 0.60 x 1.70 (copper 1.00 x 2.10) 4.18 behind the front slots; the pads start on the rear slots' centre line (4.18) and run 1.15 back; non-plated 0.65 holes 5.78 apart, 3.68 behind the front slots. The mating face is 2.60 in front of the front slots (side view), body 8.94 x 7.35. Drawn with the mating face down; rotated 180 deg. " + MAP,
  "style": "explicit", "pins": 13, "body": {"width": 8.94, "height": 7.35},
  "pads": pads(3.68, 1.15) + [slot(4.32, 3.105, 2.1, 1.7), slot(4.32, -1.075, 1.8, 1.4), hole(2.89, 2.605),
                               slot(-4.32, 3.105, 2.1, 1.7), slot(-4.32, -1.075, 1.8, 1.4), hole(-2.89, 2.605)]}
fp4500 = {"id": "USB-GCT-USB4500", "description": "GCT USB4500 USB-C receptacle, mid mount (0.8 mm offset), 12 SMD pads + 4 slotted shell legs",
  "note": "GCT drawing USB4500 rev A2 sheet 1 Recommended PCB Layout (thickness 0.80, as viewed from component side, tolerance 0.05), datasheets/connector-usb/usb4500.pdf (https://gct.co/files/drawings/usb4500.pdf; the product specification usb4500-spec.pdf prints no layout): pad centres 6.40/4.80/3.50/2.50/1.50/0.50 apart, widths 0.60 (4x) and 0.30 (8x); slots 11.24 apart; front slots 0.60 x 1.80 (copper 1.00 x 2.20) 1.60 behind the PCB edge; rear slots 0.60 x 1.40 (copper 1.00 x 1.80) 4.00 behind the front slots; pads from 4.60 to 5.70 behind the front slots (length 1.10). Board cut-out (not part of the footprint): 9.24 wide, from the PCB edge 6.20 deep (to the pads' near end), rounded inner corners. The mating face is 2.10 in front of the front slots (side view), 0.50 beyond the PCB edge; body 8.94 x 6.5 (11.24 over the shell legs). Drawn with the mating face down; rotated 180 deg. " + MAP,
  "style": "explicit", "pins": 13, "body": {"width": 8.94, "height": 6.5},
  "pads": pads(4.0, 1.1) + [slot(5.62, 2.85, 1.8, 1.4), slot(5.62, -1.15, 2.2, 1.8),
                             slot(-5.62, 2.85, 1.8, 1.4), slot(-5.62, -1.15, 2.2, 1.8)]}
names = ["GND", "VBUS", "CC1", "D+", "D-", "SBU1", "CC2", "D+", "D-", "SBU2", "VBUS", "GND", "SHIELD"]
pin = {"p": [{"number": i + 1, "name": n, "role": "inout"} for i, n in enumerate(names)]}
def dev(i, desc, note, part):
    return {"after": None, "device": {"id": i, "function": "USB-C Port", "description": desc, "drawer": ["Connector", "USB"],
            "note": note, "usb": "C", "ratedVolts": 48, "ratedAmps": 5, "pinouts": pin, "parts": [part]}}
spec = {"inlinePinouts": True,
  "datasheets": [{"id": "usb4105-spec", "url": "https://gct.co/files/specs/usb4105-spec.pdf"},
                 {"id": "usb4500-spec", "url": "https://gct.co/files/specs/usb4500-spec.pdf"}],
  "footprints": [fp4105, fp4500],
  "devices": {"connector": [
    dev("usb-gct-usb4105", "USB 2.0 Type-C receptacle, 16 contacts, right-angle SMD",
        "GCT USB4105 drawing (datasheets/connector-usb/usb4105.pdf) sheet 1: pin table (A1 GND, A4 VBUS, A5 CC1, A6 Dp1, A7 Dn1, A8 SBU1, A9 VBUS, A12 GND, B1 GND, B4 VBUS, B5 CC2, B6 Dp2, B7 Dn2, B8 SBU2, B9 VBUS, B12 GND, shell GND); Electrical: current rating 5.00 A collectively for the VBUS pins, voltage rating 48 V DC. Ordering Grid: USB4105-GF (gold flash)-A (tape and reel), shell stake 0.95 mm (blank). " + MAP[:-14],
        {"name": "USB4105-GF-A", "manufacturer": "GCT", "datasheet": "usb4105-spec", "package": "SMD right angle", "pinout": "p", "footprint": "USB-GCT-USB4105"}),
    dev("usb-gct-usb4500", "USB 2.0 Type-C receptacle, 16 contacts, mid mount",
        "GCT USB4500 drawing (datasheets/connector-usb/usb4500.pdf) sheet 1: pin table (as USB4105: A5 CC1, A6 Dp1, A7 Dn1, A8 SBU1, B5 CC2, B6 Dp2, B7 Dn2, B8 SBU2, shell GND); Electrical: current rating 5.00 A collectively for the VBUS pins, voltage rating 48 V DC; mid mount, 0.8 mm offset, in a board cut-out. Ordering Grid: USB4500-03 (3 u\" gold)-0 (without shell spring)-A (tape and reel). " + MAP[:-14],
        {"name": "USB4500-03-0-A", "manufacturer": "GCT", "datasheet": "usb4500-spec", "package": "Mid mount", "pinout": "p", "footprint": "USB-GCT-USB4500"})]}}
json.dump(spec, open('planning/plan-43-data/add/usb.json', 'w'), indent=1, ensure_ascii=False)
print('ok')
