#!/usr/bin/env python3
"""plan-42-C section 3: the pinout columns of specs.tsv against the `add` rows.

usage: python3 -I pinout-check.py [--maker <maker>] [--list] [--distinct]
                                  [--specs <specs.tsv>] [--candidates <candidates.tsv>]
                                  [--only <file with pdf TAB package TAB series lines>]

For every spec row (maker, series, pdf, package) that an `add` row of
candidates.tsv uses, checks its columns by rule (plan-42-C section 3):

  pinout        JSON `[[number, name, role], ...]`: numbered pinouts list the
                numbers 1..N once each (a position with no lead has role
                `absent`), ball pinouts a ball name `A1` once each; roles are
                power, ground, input, output, inout, nc, absent; N equals the
                pin count of DigiKey's Package / Case (specs.pin_count) unless
                pinout-count-overrides*.tsv (pdf, package, count, evidence)
                says otherwise
  regulators    JSON list of 1 to 4 objects with `type` (linear, buck, boost,
                buck-boost, charge-pump, sepic), `in`, `out`, optional
                `enable`, `enableActive` (high / low), `pg`, `adjust`,
                `adjustTo` (ground / output); every name is a pin name of the
                pinout; `in` pins have role power, `out` output, `enable`
                input, `pg` output, `adjust` input or inout and is in `analog`
  analog        JSON list of pin names (in the pinout) the voltage check skips
  pinout_adj, regulators_adj, analog_adj
                the same for the adjustable variant, required when the spec
                row has both Fixed and Adjustable `add` rows; else empty
  vref          number above 0 and at most 10, required when an element has
                `adjust`; adjust_to ground / output with it
  vin_min, dropout   numbers (V) when filled

Every pin of a pinout is a ground, nc or absent pin, or is named by an
element (in, out, enable, pg, adjust) or in `analog`. Prints one line per error,
then `<n> pinout errors`; `--list` also prints each spec row checked;
`--distinct` prints the number of distinct (maker, pinout, regulators)
combinations. Exit status 1 when there is an error.
"""
import collections
import csv
import glob
import json
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import common  # noqa: E402
import specs  # noqa: E402

ROLES = {"power", "ground", "input", "output", "inout", "nc", "absent"}
TYPES = {"linear", "buck", "boost", "buck-boost", "charge-pump", "sepic"}
KEYS = {"type", "in", "out", "enable", "enableActive", "pg", "adjust", "adjustTo"}
OVERRIDES = os.path.join(common.DATA, "pinout-count-overrides.tsv")


def overrides():
    out = {}
    for path in sorted(glob.glob(os.path.join(common.DATA, "pinout-count-overrides*.tsv"))):
        for r in csv.DictReader(open(path), delimiter="\t"):
            out[(r["pdf"], r["package"])] = int(r["count"])
    return out


def parse(text, what, errs, tag):
    if not text:
        errs.append(f"{tag}: {what} is empty")
        return None
    try:
        return json.loads(text)
    except ValueError as e:
        errs.append(f"{tag}: {what} is not JSON ({e})")
        return None


def check_pinout(pins, errs, tag, expect):
    names = {}
    if not isinstance(pins, list) or not pins:
        errs.append(f"{tag}: pinout is not a non-empty list")
        return None
    numbers = []
    for p in pins:
        if not (isinstance(p, list) and len(p) == 3):
            errs.append(f"{tag}: pin {p!r} is not [number, name, role]")
            return None
        num, name, role = p
        if role not in ROLES:
            errs.append(f"{tag}: pin {num} {name}: role {role!r}")
        numbers.append(num)
        names.setdefault(name, []).append(p)
    if all(isinstance(n, int) for n in numbers):
        if sorted(numbers) != list(range(1, len(numbers) + 1)):
            errs.append(f"{tag}: pin numbers are not 1..{len(numbers)} once each: {numbers}")
    elif any(not isinstance(n, str) for n in numbers) or len(set(numbers)) != len(numbers):
        errs.append(f"{tag}: mixed or repeated pin numbers: {numbers}")
    if expect is not None and len(pins) != expect:
        errs.append(f"{tag}: {len(pins)} pins, the package says {expect}")
    return names


def check_variant(names, regs, analog, tag, errs, adjustable):
    """Roles and coverage of one variant (fixed or adjustable)."""
    if not isinstance(regs, list) or not 1 <= len(regs) <= 4:
        errs.append(f"{tag}: regulators must be a list of 1 to 4 objects")
        return False
    if not isinstance(analog, list) or not all(isinstance(a, str) for a in analog):
        errs.append(f"{tag}: analog must be a list of names")
        return False
    used = set(analog)
    has_adjust = False
    for k, e in enumerate(regs, 1):
        if not isinstance(e, dict) or set(e) - KEYS:
            errs.append(f"{tag}: regulator {k}: keys {sorted(set(e) - KEYS) if isinstance(e, dict) else e!r}")
            continue
        if e.get("type") not in TYPES:
            errs.append(f"{tag}: regulator {k}: type {e.get('type')!r}")
        for key, roles in (("in", {"power"}), ("out", {"output"}), ("enable", {"input"}),
                           ("pg", {"output"}), ("adjust", {"input", "inout"})):
            name = e.get(key)
            if name is None:
                if key in ("in", "out"):
                    errs.append(f"{tag}: regulator {k} has no {key!r}")
                continue
            if name not in names:
                errs.append(f"{tag}: regulator {k} {key} {name!r} is not a pin name")
                continue
            used.add(name)
            for p in names[name]:
                if p[2] not in roles:
                    errs.append(f"{tag}: regulator {k} {key} pin {p[0]} {name} has role {p[2]}, "
                                f"must be {' or '.join(sorted(roles))}")
        if "enableActive" in e and (e["enableActive"] not in ("high", "low") or "enable" not in e):
            errs.append(f"{tag}: regulator {k}: enableActive needs enable and high / low")
        if "adjust" in e:
            has_adjust = True
            if e["adjust"] not in analog:
                errs.append(f"{tag}: regulator {k}: adjust pin {e['adjust']!r} is not in analog")
            if e.get("adjustTo", "ground") not in ("ground", "output"):
                errs.append(f"{tag}: regulator {k}: adjustTo {e.get('adjustTo')!r}")
        elif "adjustTo" in e:
            errs.append(f"{tag}: regulator {k}: adjustTo without adjust")
    if adjustable and not has_adjust:
        errs.append(f"{tag}: the adjustable variant has no element with an adjust pin")
    if not adjustable and has_adjust:
        errs.append(f"{tag}: the fixed variant has an adjust pin")
    for a in analog:
        if a not in names:
            errs.append(f"{tag}: analog {a!r} is not a pin name")
    for name, pins in names.items():
        for p in pins:
            if p[2] in ("ground", "nc", "absent") or name in used:
                continue
            errs.append(f"{tag}: pin {p[0]} {name} ({p[2]}) is in no regulator and not analog")
    return True


def number(text, what, tag, errs, lo, hi, needed):
    if not text:
        if needed:
            errs.append(f"{tag}: {what} is empty")
        return
    try:
        v = float(text)
    except ValueError:
        errs.append(f"{tag}: {what} {text!r} is not a number")
        return
    if not lo < v <= hi:
        errs.append(f"{tag}: {what} {v} is out of range")


def main(argv):
    maker = None
    only = None
    listing = "--list" in argv
    spec_path, cand_path = specs.SPECS, specs.CANDIDATES
    args = [a for a in argv if a != "--list"]
    distinct = "--distinct" in args
    args = [a for a in args if a != "--distinct"]
    while args:
        a = args.pop(0)
        if a == "--maker":
            maker = args.pop(0)
        elif a == "--specs":
            spec_path = args.pop(0)
        elif a == "--candidates":
            cand_path = args.pop(0)
        elif a == "--only":
            only = {tuple(l.rstrip("\n").split("\t")[:3]) for l in open(args.pop(0)) if l.strip()}
    table = specs.load(spec_path)
    rows = common.load(cand_path)
    used = collections.defaultdict(list)
    for r in rows:
        if r["reason"] == "add" and (not maker or r["maker"] == maker):
            if only is None or (r["pdf"], r["package"], r["series"]) in only:
                used[specs.key(r)].append(r)
    over = overrides()
    errs = []
    combos = set()
    for k in sorted(used):
        s = table.get(k)
        tag = f"{k[0][:3]} {os.path.basename(k[2])} | {k[3]}"
        if s is None:
            errs.append(f"{tag}: no spec row")
            continue
        # `Adjustable (Fixed)` (5 add rows) is a fixed-voltage orderable of a part that also comes adjustable.
        has_fixed = any(r["output_type"] != "Adjustable" for r in used[k])
        has_adj = any(r["output_type"] == "Adjustable" for r in used[k])
        mixed = has_fixed and has_adj
        expect = over.get((k[2], k[3]), specs.pin_count(k[3]))
        variants = []
        if has_fixed:
            variants.append(("fixed", s["pinout"], s["regulators"], s["analog"], False))
        if has_adj:
            if mixed:
                variants.append(("adj", s["pinout_adj"], s["regulators_adj"], s["analog_adj"], True))
            else:
                variants.append(("adj", s["pinout"], s["regulators"], s["analog"], True))
        if not mixed and (s["pinout_adj"] or s["regulators_adj"] or s["analog_adj"]):
            errs.append(f"{tag}: pinout_adj / regulators_adj / analog_adj filled but the row is not mixed")
        if listing:
            print("check", tag, "|", "mixed" if mixed else ("adjustable" if has_adj else "fixed"), "|", len(used[k]), "add rows")
        for name, pinout_text, regs_text, analog_text, adjustable in variants:
            vtag = f"{tag} [{name}]"
            pins = parse(pinout_text, "pinout", errs, vtag)
            regs = parse(regs_text, "regulators", errs, vtag)
            analog = parse(analog_text or "[]", "analog", errs, vtag)
            if pins is None or regs is None or analog is None:
                continue
            names = check_pinout(pins, errs, vtag, expect)
            if names is None:
                continue
            if check_variant(names, regs, analog, vtag, errs, adjustable):
                combos.add((k[0], json.dumps(pins), json.dumps(regs, sort_keys=True)))
        if has_adj:
            number(s["vref"], "vref", tag, errs, 0, 10, True)
            if s["adjust_to"] not in ("ground", "output"):
                errs.append(f"{tag}: adjust_to {s['adjust_to']!r}")
        elif s["vref"] or s["adjust_to"]:
            errs.append(f"{tag}: vref / adjust_to filled for a fixed-only row")
        number(s["vin_min"], "vin_min", tag, errs, 0, 100, False)
        number(s["dropout"], "dropout", tag, errs, -1, 10, False)
    for e in errs:
        print(e)
    print(f"{len(used)} spec rows checked; {len(errs)} pinout errors")
    if distinct:
        print(f"{len(combos)} distinct (maker, pinout, regulators) combinations")
    return 1 if errs else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
