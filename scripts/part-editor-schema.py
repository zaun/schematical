#!/usr/bin/env python3
"""Generate the Part Editor's KindSchema for kinds (plan-32-F..J).

Usage: python3 scripts/part-editor-schema.py . <kind> [<kind> ...]

For each kind, reads schematical/schematical/Parts/<kind>.json and the kind's
backend sources (KIND_FILES), infers each device and part key's control from
the shipped values, adds keys the sources accept but no shipped part uses
(EXTRA, hand-placed from reading the source), and writes
schematical/schematical/Editor/<Name>Draft.swift.
"""
import json, os, re, sys

KIND_FILES = {
    "gate": ["gate.c"],
    "dff": ["dff.c"], "jkff": ["jkff.c"],
    "mux": ["mux.c", "mux_stored.c", "seq.c"], "decoder": ["decoder.c", "seq.c"],
    "latch": ["latch.c", "seq.c"], "buffer": ["buffer.c", "buffer_reg.c", "seq.c"],
    "arith": ["arith.c", "arith_alu.c", "seq.c"],
    "counter": ["counter.c", "counter_div.c", "seq.c"], "shift": ["shift.c", "shift_special.c", "seq.c"],
    "sram": ["sram.c"], "eeprom": ["eeprom.c"], "switch": ["switch.c"], "led": ["led.c"],
    "7seg": ["display7.c"], "oscillator": ["oscillator.c"], "supply": ["supply.c"],
    "cpu": ["cpu.c"], "pio": ["pio.c", "pio_pia6521.c", "pio_via6522.c"],
    "serial": None,  # serial*.c and ns16550*.c
    "charlcd": ["charlcd.c"], "connector": ["connector.c"],
    "converter": ["converter.c", "converter_load.c", "converter_par.c"],
    "rtc": ["rtc.c", "converter_load.c", "converter_par.c"],
    "diode": ["diode.c"], "header": ["header.c"],
}

NAMES = {
    "7seg": "SevenSeg", "charlcd": "CharLcd", "jkff": "Jkff", "dff": "Dff", "rtc": "Rtc", "cpu": "Cpu",
    "pio": "Pio", "sram": "Sram",
}

GENERIC_DEVICE = {"id", "function", "description", "drawer", "note", "pinouts", "parts"}
GENERIC_PART = {"name", "package", "pinout", "flags", "footprint", "manufacturer", "datasheet",
                "connectorFootprint", "footprintChoices"}

VOLT_KEYS = {"vcc", "vccB"}
# Strings the shared loaders accept only from a fixed list (seq.c, gate.c).
ENUMS = {"active": ["high", "low"], "edge": ["rising", "falling"], "when": ["async", "clock"],
         "output": ["push-pull", "open-drain"]}
UNITS = {"inputMax": "V", "inputMaxB": "V", "volts": "V", "ratedVolts": "V", "ratedAmps": "A",
         "minHz": "Hz", "maxHz": "Hz", "rise": "ns", "fall": "ns"}


def kind_files(repo, kind):
    models = os.path.join(repo, "backend/src/models")
    if KIND_FILES[kind] is None:
        return sorted(f for f in os.listdir(models)
                      if f.endswith(".c") and (f.startswith("serial") or f.startswith("ns16550")))
    return KIND_FILES[kind]


def source_keys(repo, kind):
    keys = set()
    for name in kind_files(repo, kind):
        text = open(os.path.join(repo, "backend/src/models", name)).read()
        for body in re.findall(r"KEYS\[\]\s*=\s*\{(.*?)\}", text, re.S):
            keys.update(re.findall(r'"([^"]*)"', body))
        # A model's extra timing keys (counter.c RIPPLE_EXTRA, shift_special.c
        # FIFO_EXTRA): {{"stage", 1}, {NULL, 0}}.
        for body in re.findall(r"_EXTRA\[\]\s*=\s*\{(.*?)\};", text, re.S):
            keys.update(re.findall(r'"([^"]*)"', body))
    return keys


def label(key):
    words = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", key).split()
    text = " ".join(words)
    return text[:1].upper() + text[1:]


class Ctx:
    def __init__(self, known, pins):
        self.known = known
        self.pins = pins  # set of pin names of the value's device


def category(v):
    if isinstance(v, bool):
        return "bool"
    if isinstance(v, (int, float)):
        return "num"
    if isinstance(v, str):
        return "str"
    if isinstance(v, list):
        return "list"
    if isinstance(v, dict):
        return "dict"
    return "null"


def infer(key, items, known):
    """items: list of (value, pins). Returns a type tree."""
    cats = {}
    for v, p in items:
        cats.setdefault(category(v), []).append((v, p))
    if len(cats) > 1:
        return ("oneOf", [infer(key, cats[c], known) for c in sorted(cats)])
    cat = next(iter(cats))
    vals = cats[cat]
    if cat == "str":
        if key == "note":
            return ("note",)
        if all(v in p for v, p in vals) and any(p for _, p in vals):
            return ("pin",)
        distinct = sorted({v for v, _ in vals})
        if key in ENUMS and set(distinct) <= set(ENUMS[key]):
            return ("choice", ENUMS[key])
        if len(distinct) <= 24 and all(len(v) <= 32 for v in distinct):
            return ("suggested", distinct)
        return ("text",)
    if cat == "num":
        if key in UNITS or not all(float(v).is_integer() for v, _ in vals):
            return ("number", UNITS.get(key, ""))
        return ("integer",)
    if cat == "bool":
        return ("bool",)
    if cat == "list":
        elements = [(e, p) for v, p in vals for e in v]
        if key in VOLT_KEYS and all(len(v) == 2 and all(category(e) == "num" for e in v) for v, _ in vals):
            return ("voltRange",)
        if not elements:
            return ("names",)
        ecats = {category(e) for e, _ in elements}
        if ecats == {"str"}:
            if all(e in p for e, p in elements):
                return ("pins",)
            return ("names",)
        if ecats == {"num"}:
            return ("numbers", UNITS.get(key, ""))
        if ecats == {"dict"} and all(set(e) <= known for e, _ in elements):
            return ("list", fields([(e, p) for e, p in elements], known))
        return ("listOf", infer(key, elements, known))
    if cat == "dict":
        if all(set(v) <= known for v, _ in vals):
            return ("object", fields(vals, known))
        entries = [(e, p) for v, p in vals for e in v.values()]
        if entries and all(category(e) == "dict" and set(e) <= known for e, _ in entries):
            return ("map", "Name", fields(entries, known))
        return ("valueMap", "Name", infer(key, entries, known) if entries else ("text",))
    return ("json",)


def fields(objects, known):
    """objects: list of (dict, pins). Returns list of (key, type, required)."""
    keys = []
    for o, _ in objects:
        for k in o:
            if k not in keys:
                keys.append(k)
    out = []
    for k in sorted(keys):
        present = [(o[k], p) for o, p in objects if k in o]
        out.append((k, infer(k, present, known), len(present) == len(objects)))
    return out


def swift_type(t, emit):
    tag = t[0]
    if tag in ("text", "note", "integer", "bool", "pin", "pins", "names", "voltRange", "json"):
        return "." + tag
    if tag == "number":
        return '.number(unit: "%s")' % t[1]
    if tag == "numbers":
        return '.numbers(unit: "%s")' % t[1]
    if tag == "suggested":
        return ".suggested([%s])" % ", ".join(json.dumps(v) for v in t[1])
    if tag == "choice":
        return ".choice([%s])" % ", ".join(json.dumps(v) for v in t[1])
    if tag in ("object", "list"):
        return ".%s([%s])" % (tag, ", ".join(emit(f) for f in t[1]))
    if tag == "map":
        return '.map(keyLabel: "%s", [%s])' % (t[1], ", ".join(emit(f) for f in t[2]))
    if tag == "valueMap":
        return '.valueMap(keyLabel: "%s", %s)' % (t[1], swift_type(t[2], emit))
    if tag == "listOf":
        return ".listOf(%s)" % swift_type(t[1], emit)
    if tag == "oneOf":
        return ".oneOf([%s])" % ", ".join(swift_type(x, emit) for x in t[1])
    raise ValueError(t)


def keys_of(t):
    tag = t[0]
    out = set()
    if tag in ("object", "list"):
        for k, sub, _ in t[1]:
            out.add(k)
            out |= keys_of(sub)
    elif tag == "map":
        for k, sub, _ in t[2]:
            out.add(k)
            out |= keys_of(sub)
    elif tag in ("valueMap",):
        out |= keys_of(t[2])
    elif tag == "listOf":
        out |= keys_of(t[1])
    elif tag == "oneOf":
        for x in t[1]:
            out |= keys_of(x)
    return out


def generate(repo, kind, extra):
    data = json.load(open(os.path.join(repo, "schematical/schematical/Parts/%s.json" % kind)))
    known = source_keys(repo, kind)
    devices, parts = [], []
    for d in data["devices"]:
        pins = {p["name"] for po in d["pinouts"].values() for p in po}
        devices.append(({k: v for k, v in d.items() if k not in GENERIC_DEVICE}, pins))
        for p in d["parts"]:
            parts.append(({k: v for k, v in p.items() if k not in GENERIC_PART}, pins))
    variant_key = None
    for vk in ("model", "type"):
        if all(vk in d for d, _ in devices) and all(isinstance(d[vk], str) for d, _ in devices):
            variant_key = vk
            break
    dev_fields = fields(devices, known)
    part_fields = fields(parts, known)
    if variant_key:
        models = sorted({d[variant_key] for d, _ in devices})
        dev_fields = [(k, ("choice", models) if k == variant_key else t, r) for k, t, r in dev_fields]
    # Variants per top-level device key.
    variants = {}
    if variant_key:
        allv = {d[variant_key] for d, _ in devices}
        for k, _, _ in dev_fields:
            if k == variant_key:
                continue
            seen = {d[variant_key] for d, _ in devices if k in d}
            if seen != allv:
                variants[k] = sorted(seen)
    for entry in extra.get(kind, []):
        where, key, type_, path = entry[:4]
        target = dev_fields if where == "device" else part_fields
        add_extra(target, path, key, type_)
        if len(entry) > 4 and entry[4] and where == "device" and not path:
            variants[key] = sorted(entry[4])

    counter = [0]
    decls = []

    def emit(f):
        k, t, req = f
        counter[0] += 1
        name = "f%d" % counter[0]
        expr = swift_type(t, emit)
        args = 'key: %s, label: %s, type: %s' % (json.dumps(k), json.dumps(label(k)), expr)
        if req:
            args += ", required: true"
        decls.append("    static let %s: Field = Field(%s)" % (name, args))
        return name

    top_dev = []
    for f in dev_fields:
        name = emit(f)
        if f[0] in variants:
            decls[-1] = decls[-1][:-1] + ", variants: [%s])" % ", ".join(json.dumps(v) for v in variants[f[0]])
        top_dev.append(name)
    top_part = [emit(f) for f in part_fields]

    cls = NAMES.get(kind, kind[:1].upper() + kind[1:]) + "Draft"
    lines = [
        "import Foundation",
        "",
        "// Generated by scripts/part-editor-schema.py from backend/src/models (%s) and" % ", ".join(kind_files(repo, kind)),
        "// Parts/%s.json; see plan-32-F § Corrections. Edit by hand freely." % kind,
        "",
        "/// The %s kind's keys (plan-32-F … J): one field per device and part key, nested" % kind,
        "/// keys included, each with its control.",
        "enum %s {" % cls,
    ]
    lines += decls
    lines += [
        "",
        "    static let schema = KindSchema(",
        '        kind: %s,' % json.dumps(kind),
        "        variantKey: %s," % (json.dumps(variant_key) if variant_key else "nil"),
        "        device: [%s]," % ", ".join(top_dev),
        "        part: [%s]" % ", ".join(top_part),
        "    )",
        "}",
        "",
    ]
    out = os.path.join(repo, "schematical/schematical/Editor/%s.swift" % cls)
    open(out, "w").write("\n".join(lines))
    covered = set(GENERIC_DEVICE | GENERIC_PART | {"number", "ball", "name", "role", "group", "side"})
    for k, t, _ in dev_fields + part_fields:
        covered.add(k)
        covered |= keys_of(t)
    missing = sorted(known - covered)
    print("%s: %d devices, %d parts, variant %s, %d source keys, missing %s -> %s" % (
        kind, len(devices), len(parts), variant_key, len(known), missing, out))


def add_extra(target, path, key, type_):
    """Adds key (type_ a type tree) under path (list of keys) in a field list."""
    if not path:
        for k, _, _ in target:
            if k == key:
                return
        target.append((key, type_, False))
        target.sort(key=lambda f: f[0])
        return
    for i, (k, t, req) in enumerate(target):
        if k == path[0]:
            sub = None
            if t[0] in ("object", "list"):
                sub = t[1]
            elif t[0] == "map":
                sub = t[2]
            if sub is None:
                raise ValueError("cannot add under %s" % path)
            add_extra(sub, path[1:], key, type_)
            return
    raise ValueError("no %s" % path)


EXTRA = {}
_extra = os.path.join(os.path.dirname(os.path.abspath(__file__)), "part-editor-schema-extra.py")
if os.path.exists(_extra):
    exec(open(_extra).read())

if __name__ == "__main__":
    repo = sys.argv[1]
    for kind in sys.argv[2:]:
        generate(repo, kind, EXTRA)
