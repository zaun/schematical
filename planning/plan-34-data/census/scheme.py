"""plan-34-B § 3.1: an ordering-code regex's field check.

A spec row's `mpn_regex` is anchored over the whole part number and names
the fields it checks with named groups:

  size    the size code; agrees when it is a whole token of DigiKey's
          Package / Case (`0402` ~ `0402 (1005 Metric)`, `1218` ~ `Wide 1812
          (4532 Metric), 1218`), after mapping through the spec row's
          `sizemap=<code>:<size>,…` entry in fields_checked
          (Viking `03:0603`)
  tol     the tolerance letter (F ±1%, J ±5%, …) against DigiKey's
          Tolerance; a jumper's (Tolerance `Jumper`) letter is not checked
  rkm     a value in R/K/M notation (`4K7` 4700, `100R` 100, `0R` 0, `4R70`
          4.7) against DigiKey's ohms
  eia3    three digits, the last a power of ten (`472` 4700; `R47` 0.47)
  eia4    four digits, the last a power of ten (`4701` 4700; `47R0` 47)
  jumper  matched text means 0 Ω (a jumper's value field, e.g. `000`)

`fields_checked` in specs.tsv names the groups, e.g. `size,tol,rkm`, then
any `;sizemap=…`.
"""
import re

TOL = {"A": 0.05, "B": 0.1, "C": 0.25, "D": 0.5, "F": 1, "G": 2, "J": 5, "K": 10, "M": 20}


def rkm(code):
    m = re.fullmatch(r"(\d*)([RKMG])(\d*)", code)
    if not m or not (m.group(1) or m.group(3)):
        return None
    scale = {"R": 1, "K": 1e3, "M": 1e6, "G": 1e9}[m.group(2)]
    return float(f"{m.group(1) or '0'}.{m.group(3) or '0'}") * scale


def eia(code, digits):
    if "R" in code:
        return rkm(code)
    if not re.fullmatch(r"\d{%d}" % digits, code):
        return None
    return float(code[:-1]) * 10 ** int(code[-1])


def tolerance_percent(text):
    m = re.match(r"±([\d.]+)%", text or "")
    return float(m.group(1)) if m else None


def size_map(fields):
    for part in (fields or "").split(";"):
        if part.startswith("sizemap="):
            return dict(x.split(":", 1) for x in part[len("sizemap="):].split(","))
    return {}


def check(match, row, fields=""):
    """The name of the first field that disagrees with DigiKey, or None."""
    g = dict(match.groupdict())
    if g.get("size") is not None:
        g["size"] = size_map(fields).get(g["size"], g["size"])
    ohms = float(row["ohms"]) if row["ohms"] else None
    jumper = row["tolerance"] == "Jumper"
    if g.get("size") is not None and g["size"] not in re.split(r"[^0-9A-Za-z]+", row["package"]):
        return "size"
    if g.get("tol") is not None and not jumper:
        if TOL.get(g["tol"]) != tolerance_percent(row["tolerance"]):
            return "tolerance"
    value = None
    if g.get("rkm") is not None:
        value = rkm(g["rkm"])
    elif g.get("eia3") is not None:
        value = eia(g["eia3"], 3)
    elif g.get("eia4") is not None:
        value = eia(g["eia4"], 4)
    elif g.get("jumper") is not None:
        value = 0.0
    if any(g.get(k) is not None for k in ("rkm", "eia3", "eia4", "jumper")):
        if value is None or ohms is None or abs(value - ohms) > 1e-6 * max(1.0, ohms):
            return "value"
    return None


def verdict(row, regex, fields=""):
    """('add', '') or ('skip', reason) for a row under a spec row's regex."""
    m = re.fullmatch(regex, row["mpn"])
    if not m:
        return "skip", "not in the datasheet's ordering table"
    bad = check(m, row, fields)
    if bad:
        return "skip", f"ordering code disagrees with DigiKey ({bad})"
    return "add", ""
