"""plan-36-B § 3.2: the pad-name -> role table.

role(name, kind) -> one of input, output, inout, power, ground, nc, or None
(a name outside the table: pinout-lint.py fails on it). `kind` is "avr",
"esp32" (the module table's Type column decides) or "rp2040".
"""
import re

PORT = re.compile(r"^P[A-R][0-9]$")

AVR = {
    "inout": {"D+", "D-", "DM", "DP", "PDI", "PDI_DATA", "UPDI"},
    "input": {"RESET", "PEN", "AREF", "REF", "XTAL1", "TOSC1", "ADC6", "ADC7", "VBUS"},
    "output": {"XTAL2", "TOSC2", "UCAP", "CAPH", "CAPL", "LCDCAP", "BIAS1", "BIAS2",
               "COM0", "COM1", "COM2", "COM3"} | {f"SEG{i}" for i in range(24)},
    "power": {"VCC", "AVCC", "AVDD", "VDD", "VDD0", "VDD1", "VDD2", "VDDIO2", "UVCC", "VUSB", "VBAT", "VLCD"},
    "ground": {"GND", "GND0", "GND1", "GND2", "AGND", "UGND"},
    "nc": {"NC", "DNC"},
}

RP2040 = {
    "input": {"XIN", "RUN", "SWCLK", "TESTEN"},
    "output": {"XOUT", "VREG_VOUT"},
    "power": {"IOVDD", "DVDD", "USB_VDD", "ADC_AVDD", "VREG_VIN"},
    "ground": {"GND"},
    "inout": {"SWDIO", "USB_DP", "USB_DM"},
}

ESP_TYPE = {"I/O": "inout", "I": "input", "O": "output", "P": "power", "-": "nc"}


def role(name, kind="avr", esp_type=None):
    if kind == "esp32":
        if name == "GND":
            return "ground"
        return ESP_TYPE.get(esp_type)
    if kind == "rp2040":
        if re.match(r"^GPIO\d+$", name) or name.startswith("QSPI_"):
            return "inout"
        for r, names in RP2040.items():
            if name in names:
                return r
        return None
    if PORT.match(name):
        return "inout"
    for r, names in AVR.items():
        if name in names:
            return r
    return None
