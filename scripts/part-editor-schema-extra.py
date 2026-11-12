# Keys the backend accepts that no shipped part uses, placed by reading the
# sources: (where, key, type, path under the device/part object, model variants).
OUTPUT = ("choice", ["push-pull", "open-drain"])
CV_EXTRA = [
    ("device", "selectPin", ("pin",), ["bus"]),
    ("device", "spiLevel", ("integer",), ["bus"]),
    ("device", "selectSampled", ("json",), ["bus"]),
    ("device", "spi", ("object", [("cs", ("pin",), False), ("sclk", ("pin",), True), ("mosi", ("pin",), False),
                                  ("miso", ("pin",), False), ("mode", ("integer",), False)]), ["bus"]),
    ("device", "i2c", ("object", [("scl", ("pin",), True), ("sda", ("pin",), True)]), ["bus"]),
    ("device", "wordBits", ("integer",), ["protocol", "commands"]),
]
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
    # converter_load.c load_bus, style "i2c-spi": "selectPin" (a pin), "spiLevel"
    # (0 or 1), "spi" {cs, sclk, mosi, miso, mode} and "i2c" {scl, sda};
    # "selectSampled" is in its key list but never read (any JSON). load_command:
    # "wordBits". converter and rtc share both loaders.
    "converter": CV_EXTRA,
    "rtc": CV_EXTRA,
    # cpu.c / pio.c PART_KEYS "vcc", "inputMax" (kind_part_voltage over the
    # grade's) and the grades' EXTRA "inputMax".
    "cpu": [("part", "vcc", ("voltRange",), []), ("part", "inputMax", ("number", "V"), []),
            ("device", "inputMax", ("number", "V"), ["grades"])],
    "pio": [("part", "vcc", ("voltRange",), []), ("part", "inputMax", ("number", "V"), []),
            ("device", "inputMax", ("number", "V"), ["grades"])],
    # serial.c PART_KEYS: "vcc", "inputMax", and acia6551's "ctsA" (a bool).
    "serial": [("part", "vcc", ("voltRange",), []), ("part", "inputMax", ("number", "V"), []),
               ("part", "ctsA", ("bool",), [])],
    # charlcd.c DEVICE_KEYS and PART_KEYS "vcc", "inputMax".
    "charlcd": [("device", "inputMax", ("number", "V"), []), ("part", "vcc", ("voltRange",), []),
                ("part", "inputMax", ("number", "V"), [])],
    # oscillator.c DEVICE_KEYS and PART_KEYS "inputMax".
    "oscillator": [("device", "inputMax", ("number", "V"), []), ("part", "inputMax", ("number", "V"), [])],
}
