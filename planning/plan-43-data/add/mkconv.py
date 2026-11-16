"""Write planning/plan-43-data/add/converters.json (plan-43-D phase 2): MCP3428, DS4432 and AD5621, transcribed
from their datasheets after the library's ADS1112, pointer-protocol I2C devices and DAC7311."""
import json

def pins(*rows):
    out = []
    for i, (name, role, *rest) in enumerate(rows):
        p = {"number": i + 1, "name": name, "role": role}
        if rest: p["activeLow"] = True
        out.append(p)
    return out

table = [None] * 16
table[0], table[1], table[4], table[5] = 0x68, 0x6C, 0x6A, 0x6E   # index = tie(Adr0) + 4 * tie(Adr1); 0 = low, 1 = high
mcp3428 = {
 "id": "MCP3428", "function": "ADC",
 "description": "16-bit, 4-channel delta-sigma ADC with I2C interface, PGA and on-board reference",
 "drawer": ["Interface"],
 "note": "22226a.pdf (Microchip DS22226A, MCP3426/7/8): p.6 electrical characteristics (VDD 2.7-5.5 V), p.11 Table 3-1 pins (MCP3428 TSSOP: 1 CH1+, 2 CH1-, 3 CH2+, 4 CH2-, 5 VSS, 6 VDD, 7 SDA, 8 SCL, 9 Adr0, 10 Adr1, 11 CH3+, 12 CH3-, 13 CH4+, 14 CH4-), p.18 Register 5-1 configuration register (power-on 90h: RDY 1, channel 1, continuous, 240 SPS, gain 1), p.20-21 device code 1101 and Table 5-3 address bits (Adr0, Adr1: low/low 000, high/low 100, low/high 010, high/high 110; the floating states give 001, 101, 011, 111 and cannot be told from a tie, so they have no table entry), p.21-22 write (one configuration byte) and read (data bytes, then the configuration byte), p.22 general call reset, p.53 product identification (MCP3428-E/ST). Left out: the 12- and 14-bit output widths of the 240 and 60 SPS rates (the result is always 16 bits), the PGA, general call latch and conversion; S1-S0 = 11 converts at 15 SPS.",
 "pinouts": {"tssop14": pins(("CH1+", "input"), ("CH1-", "input"), ("CH2+", "input"), ("CH2-", "input"), ("VSS", "ground"), ("VDD", "power"), ("SDA", "inout"), ("SCL", "input"), ("Adr0", "input"), ("Adr1", "input"), ("CH3+", "input"), ("CH3-", "input"), ("CH4+", "input"), ("CH4-", "input"))},
 "type": "adc", "bits": 16, "channels": 4, "codeFormat": "twos",
 "vcc": [2.7, 5.5],
 "analog": ["CH1+", "CH1-", "CH2+", "CH2-", "CH3+", "CH3-", "CH4+", "CH4-"],
 "bus": {"style": "i2c", "scl": "SCL", "sda": "SDA", "address": ["Adr0", "Adr1"], "table": table},
 "registers": [
  {"name": "OUTPUT", "width": 16, "reset": 0, "readOnly": True, "result": True},
  {"name": "CONFIG", "width": 8, "reset": 0x90, "readOnly": 128,
   "fields": [{"name": "RDY", "bits": [7, 7]}, {"name": "C", "bits": [6, 5]}, {"name": "OC", "bits": [4, 4]}, {"name": "S", "bits": [3, 2]}, {"name": "G", "bits": [1, 0]}],
   "note": "reset 90h; bit 7 reads the ready status (0 = the output register holds a new result)"}],
 "protocol": {"frame": "fixed",
  "commands": [{"name": "Write Configuration", "in": "dddddddd", "action": "write", "register": "CONFIG", "data": "d",
                "note": "one configuration byte after the address byte (Figure 5-3)"}],
  "read": "[OUTPUT][CONFIG]", "idle": "1", "generalCall": "reset"},
 "conversion": {"time": {"field": "CONFIG.S", "values": {"0": 4166667, "1": 16666667, "2": 66666667, "3": 66666667}},
  "start": ["write:CONFIG.RDY=1"], "mode": {"field": "CONFIG.OC", "continuous": 1},
  "done": {"CONFIG.RDY": 0}, "onRead": {"CONFIG.RDY": 1}, "channel": "CONFIG.C"},
 "parts": [{"name": "MCP3428-E/ST", "manufacturer": "Microchip Technology", "datasheet": "22226a", "package": "TSSOP", "pinout": "tssop14", "footprint": "TSSOP-14"}],
}

ds4432 = {
 "id": "DS4432", "function": "DAC",
 "description": "Dual 7-bit sink/source current DAC with I2C interface",
 "drawer": ["Interface"],
 "note": "DS4432.pdf (Maxim 19-4744): p.1 ordering information (DS4432U+, 8 uSOP) and pin configuration, p.2 recommended operating conditions (VCC 2.7-5.5 V), p.4 pin description (1 SDA, 2 SCL, 3 FS1, 4 GND, 5 FS0, 6 OUT0, 7 OUT1, 8 VCC), p.6 memory organization (F8h OUT0, F9h OUT1; each register: bit 7 sign, 1 = source, bits 6-0 the current setting; zero current at power-up), I2C slave address 90h (7-bit 48h), p.8 write (memory address, data) and read (set the address, then read). The sign bit is stored; an output's code is the 7-bit magnitude.",
 "pinouts": {"usop8": pins(("SDA", "inout"), ("SCL", "input"), ("FS1", "input"), ("GND", "ground"), ("FS0", "input"), ("OUT0", "output"), ("OUT1", "output"), ("VCC", "power"))},
 "type": "dac", "bits": 7, "channels": 2,
 "vcc": [2.7, 5.5],
 "analog": ["FS1", "FS0", "OUT0", "OUT1"],
 "bus": {"style": "i2c", "scl": "SCL", "sda": "SDA", "address": [], "table": [72]},
 "registers": [
  {"name": "OUT0", "address": 248, "width": 8, "reset": 0, "fields": [{"name": "SIGN", "bits": [7, 7]}, {"name": "DATA", "bits": [6, 0]}]},
  {"name": "OUT1", "address": 249, "width": 8, "reset": 0, "fields": [{"name": "SIGN", "bits": [7, 7]}, {"name": "DATA", "bits": [6, 0]}]}],
 "protocol": {"frame": "fixed",
  "commands": [{"name": "Memory address", "in": "aaaaaaaa", "action": "pointer", "address": "a",
                "note": "the first write byte is the memory address (F8h or F9h); the next byte writes that register, a read returns it"}],
  "read": "pointer"},
 "dac": {"input": ["OUT0", "OUT1"], "output": ["OUT0", "OUT1"], "clearCode": 0, "ldac": "sync"},
 "parts": [{"name": "DS4432U+", "manufacturer": "Analog Devices", "datasheet": "ds4432", "package": "VSSOP", "pinout": "usop8", "footprint": "VSSOP-8-DGK"}],
}

ad5621 = {
 "id": "AD5621", "function": "DAC",
 "description": "12-bit nanoDAC with SPI interface, rail-to-rail voltage output",
 "drawer": ["Interface"],
 "note": "AD5601_5611_5621.pdf (Analog Devices, rev. I): p.4 specifications (VDD 2.7-5.5 V), p.6 pin function descriptions (SC70: 1 SYNC, 2 SCLK, 3 SDIN, 4 VDD, 5 GND, 6 VOUT), p.14 serial interface (16-bit shift register, data clocked in on the falling edge of SCLK, register updated on the 16th falling edge; SYNC high earlier aborts the write) and Figure 41 input register (DB15-DB14 PD1-PD0, DB13-DB2 the 12 data bits, DB1-DB0 ignored), p.16 Table 6 power-down modes (stored, not acted on), power-on reset to zero scale, p.21 ordering guide (AD5621AKSZ-REEL7, KS-6).",
 "pinouts": {"sc70": pins(("SYNC", "input", 1), ("SCLK", "input"), ("SDIN", "input"), ("VDD", "power"), ("GND", "ground"), ("VOUT", "output"))},
 "type": "dac", "bits": 12, "channels": 1,
 "vcc": [2.7, 5.5],
 "analog": ["VOUT"],
 "bus": {"style": "spi", "cs": "SYNC", "sclk": "SCLK", "mosi": "SDIN", "mode": 2},
 "registers": [
  {"name": "DAC", "width": 12, "reset": 0, "note": "DAC register; power-on reset fills it with zeros"},
  {"name": "CTRL", "width": 2, "reset": 0, "fields": [{"name": "PD", "bits": [1, 0]}],
   "note": "internal: PD1/PD0 mode bits of the last valid frame (Table 6; 00 normal operation at power on)"}],
 "protocol": {"frame": "fixed",
  "commands": [{"name": "WRITE", "in": "pp dddddddddddd xx", "action": "write", "channel": 0, "data": "d", "set": {"CTRL.PD": "p"},
                "note": "DB15-DB14 PD1/PD0, then the data MSB first, then two ignored bits; executed on the 16th SCLK falling edge; SYNC high earlier aborts the frame"}]},
 "dac": {"input": ["DAC"], "output": ["DAC"], "clearCode": 0, "ldac": "sync"},
 "parts": [{"name": "AD5621AKSZ-REEL7", "manufacturer": "Analog Devices", "datasheet": "ad5601_5611_5621", "package": "SC-70", "pinout": "sc70", "footprint": "SC70-6-ADI-KS-6"}],
}

spec = {
 "datasheets": [{"id": "22226a", "url": "https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/22226a.pdf"},
                {"id": "ds4432", "url": "https://www.analog.com/media/en/technical-documentation/data-sheets/DS4432.pdf"},
                {"id": "ad5601_5611_5621", "url": "https://www.analog.com/media/en/technical-documentation/data-sheets/AD5601_5611_5621.pdf"}],
 "footprints": [
  {"id": "SC70-6-ADI-KS-6", "description": "6-pin SC70 (Analog Devices KS-6), 1.25 x 2.0 mm body, 0.65 mm pitch",
   "note": "Analog Devices KS-6 outline, JEDEC MO-203-AB (R2; no land pattern printed), datasheets/dac/AD5601_5611_5621.pdf p.20 Figure 54: L 0.26-0.46 -> pad length 1.26; b 0.15-0.30 -> pad width 0.35; 1.80-2.40 over the leads -> span 2.40 + 1.0 - 1.26 = 2.14; 0.65 BSC; body 1.25 (1.15-1.35) x 2.00 (1.80-2.20). Not SC70-6 (TI: 0.9 x 0.4 pads, span 2.2)",
   "style": "dual", "pins": 6, "body": {"width": 1.25, "height": 2.0}, "pitch": 0.65, "span": 2.14, "pad": {"width": 1.26, "height": 0.35, "shape": "rect"}}],
 "devices": {"converter": [mcp3428, ds4432, ad5621]}}
json.dump(spec, open('planning/plan-43-data/add/converters.json', 'w'), indent=1, ensure_ascii=False)
print('ok')
