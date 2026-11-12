# Keys the backend accepts that no shipped part uses, placed by reading the
# sources: (where, key, type, path under the device/part object, model variants).
OUTPUT = ("choice", ["push-pull", "open-drain"])
EXTRA = {
    # mux.c DEVICE_KEYS / seq_output_style: every model takes "output".
    "mux": [("device", "output", OUTPUT, [])],
    # decoder.c UNIT_KEYS (binary units) "clock" (seq_optional_name, a pin);
    # SEG7_KEYS "phase" (seq_optional_name, a pin).
    "decoder": [("device", "clock", ("pin",), ["units"]),
                ("device", "phase", ("pin",), [], ["seg7"])],
    # arith.c PART_KEYS and seq_timing's BASE_KEYS take "inputMax".
    "arith": [("part", "inputMax", ("number", "V"), []),
              ("device", "inputMax", ("number", "V"), ["timing"])],
    # sram.c / eeprom.c PART_KEYS "vcc", "inputMax" (kind_part_voltage), and each
    # grade's kind_voltage.
    # TIMING_NAMES entries no shipped grade uses (each a time in ns).
    "sram": [("part", "vcc", ("voltRange",), []), ("part", "inputMax", ("number", "V"), []),
             ("device", "inputMax", ("number", "V"), ["grades"])]
            + [("device", t, ("number", "ns"), ["grades"]) for t in ["tas", "tdh", "twr"]],
    "eeprom": [("part", "vcc", ("voltRange",), []), ("part", "inputMax", ("number", "V"), []),
               ("device", "inputMax", ("number", "V"), ["grades"])]
              + [("device", t, ("number", "ns"), ["grades"]) for t in ["tas", "tch", "tcs", "toes", "toh"]],
    # oscillator.c DEVICE_KEYS and PART_KEYS "inputMax".
    "oscillator": [("device", "inputMax", ("number", "V"), []), ("part", "inputMax", ("number", "V"), [])],
}
