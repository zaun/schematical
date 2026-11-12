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
}
