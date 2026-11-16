"""Write planning/plan-43-data/add/rtc.json (plan-43-D phase 1): DS3231M and PCF8563, transcribed from their datasheets."""
import json

def reg(name, addr, reset, fields=None, **kw):
    r = {"name": name, "address": addr, "width": 8, "reset": reset}
    if fields:
        r["fields"] = [{"name": n, "bits": [hi, lo]} for n, hi, lo in fields]
    r.update(kw)
    return r

def pins(*rows):
    out = []
    for i, (name, role, *rest) in enumerate(rows):
        p = {"number": i + 1, "name": name, "role": role}
        if rest: p["activeLow"] = True
        out.append(p)
    return out

POINTER = {"frame": "fixed", "commands": [{"name": "Register address", "in": "aaaaaaaa", "action": "pointer", "address": "a", "increment": 1,
           "note": "the first write byte is the register address; further bytes write consecutive registers, reads continue from it"}], "read": "pointer"}

ds3231m = {
 "id": "DS3231M", "function": "RTC",
 "description": "I2C real-time clock with MEMS resonator, two alarms and battery backup input",
 "drawer": ["Interface"],
 "note": "DS3231M.pdf (Maxim 19-5312): p.1 ordering information (DS3231MZ+ 8 SO; +TRL is its tape and reel), p.2 recommended operating conditions (VCC 2.3-5.5 V), p.7-8 pin description (8 SO: 1 32KHZ, 2 VCC, 3 INT/SQW, 4 RST, 5 GND, 6 VBAT, 7 SDA, 8 SCL), p.11 Table 2 timekeeping registers, p.13-15 Control (0Eh: 1Ch at power-on, INTCN = 1) and Status (0Fh: OSF and EN32KHZ 1 at power-on), p.16-18 I2C (slave address 1101000, register pointer with auto-increment). Modelled: time and calendar in 24-hour mode, the century bit, both alarms on INT/SQW (open drain, date matching). Left out: 12-hour mode (bit 5 of the hours registers is both the 20-hour digit and AM/PM; the 12/24 bits are stored only), day-of-week alarm matching (DY/DT is stored only), the 1 Hz square wave (INTCN = 0), the 32 kHz output, RST, the temperature registers (read 0), the aging offset, and EOSC (it stops the oscillator only on VBAT; the model has no battery supply, so the clock always runs). Alarm 2 has no seconds register: it matches during second 00. The day-of-week register is user-defined; it starts at 7 for Saturday 2000-01-01 (1 = Sunday).",
 "pinouts": {"so8": pins(("32KHZ", "output"), ("VCC", "power"), ("INT/SQW", "output", 1), ("RST", "inout", 1), ("GND", "ground"), ("VBAT", "power"), ("SDA", "inout"), ("SCL", "input"))},
 "vcc": [2.3, 5.5],
 "analog": ["VBAT"],
 "bus": {"style": "i2c", "scl": "SCL", "sda": "SDA", "address": [], "table": [104]},
 "registers": [
  reg("SECONDS", 0, 0, [("ZERO", 7, 7), ("SECONDS", 6, 0)]),
  reg("MINUTES", 1, 0, [("ZERO", 7, 7), ("MINUTES", 6, 0)]),
  reg("HOURS", 2, 0, [("ZERO", 7, 7), ("H12", 6, 6), ("HOURS", 5, 0)]),
  reg("DAY", 3, 7, [("ZERO", 7, 3), ("DAY", 2, 0)]),
  reg("DATE", 4, 1, [("ZERO", 7, 6), ("DATE", 5, 0)]),
  reg("MONTH", 5, 1, [("CENTURY", 7, 7), ("ZERO", 6, 5), ("MONTH", 4, 0)]),
  reg("YEAR", 6, 0, [("YEAR", 7, 0)]),
  reg("A1_SECONDS", 7, 0, [("A1M1", 7, 7), ("SECONDS", 6, 0)]),
  reg("A1_MINUTES", 8, 0, [("A1M2", 7, 7), ("MINUTES", 6, 0)]),
  reg("A1_HOURS", 9, 0, [("A1M3", 7, 7), ("H12", 6, 6), ("HOURS", 5, 0)]),
  reg("A1_DATE", 10, 0, [("A1M4", 7, 7), ("DYDT", 6, 6), ("DATE", 5, 0)]),
  reg("A2_MINUTES", 11, 0, [("A2M2", 7, 7), ("MINUTES", 6, 0)]),
  reg("A2_HOURS", 12, 0, [("A2M3", 7, 7), ("H12", 6, 6), ("HOURS", 5, 0)]),
  reg("A2_DATE", 13, 0, [("A2M4", 7, 7), ("DYDT", 6, 6), ("DATE", 5, 0)]),
  reg("CONTROL", 14, 0x1C, [("EOSC", 7, 7), ("BBSQW", 6, 6), ("CONV", 5, 5), ("NA", 4, 3), ("INTCN", 2, 2), ("A2IE", 1, 1), ("A1IE", 0, 0)]),
  reg("STATUS", 15, 0x88, [("OSF", 7, 7), ("ZERO", 6, 4), ("EN32KHZ", 3, 3), ("BSY", 2, 2), ("A2F", 1, 1), ("A1F", 0, 0)]),
  reg("AGING", 16, 0),
  reg("TEMP_MSB", 17, 0, readOnly=True, note="temperature, not modelled; reads 00h"),
  reg("TEMP_LSB", 18, 0, readOnly=True, note="temperature, not modelled; reads 00h"),
 ],
 "protocol": POINTER,
 "clock": {
  "fields": [
   {"field": "SECONDS.SECONDS", "unit": "second", "format": "bcd"},
   {"field": "MINUTES.MINUTES", "unit": "minute", "format": "bcd"},
   {"field": "HOURS.HOURS", "unit": "hour", "format": "bcd"},
   {"field": "DAY.DAY", "unit": "weekday", "first": 1},
   {"field": "DATE.DATE", "unit": "date", "format": "bcd"},
   {"field": "MONTH.MONTH", "unit": "month", "format": "bcd"},
   {"field": "YEAR.YEAR", "unit": "year", "format": "bcd", "base": 2000},
   {"field": "MONTH.CENTURY", "unit": "century", "format": "binary"}],
  "update": "none",
  "alarms": [
   {"match": [{"field": "A1_SECONDS.SECONDS", "unit": "second", "mask": "A1_SECONDS.A1M1"},
              {"field": "A1_MINUTES.MINUTES", "unit": "minute", "mask": "A1_MINUTES.A1M2"},
              {"field": "A1_HOURS.HOURS", "unit": "hour", "mask": "A1_HOURS.A1M3"},
              {"field": "A1_DATE.DATE", "unit": "date", "mask": "A1_DATE.A1M4"}],
    "flag": "STATUS.A1F", "enable": "CONTROL.A1IE", "pin": {"pin": "INT/SQW", "active": 0, "openDrain": True}},
   {"match": [{"field": "A2_MINUTES.MINUTES", "unit": "minute", "mask": "A2_MINUTES.A2M2"},
              {"field": "A2_HOURS.HOURS", "unit": "hour", "mask": "A2_HOURS.A2M3"},
              {"field": "A2_DATE.DATE", "unit": "date", "mask": "A2_DATE.A2M4"}],
    "flag": "STATUS.A2F", "enable": "CONTROL.A2IE", "pin": {"pin": "INT/SQW", "active": 0, "openDrain": True}}]},
 "parts": [{"name": "DS3231MZ+TRL", "manufacturer": "Analog Devices", "datasheet": "ds3231m", "package": "SOIC", "pinout": "so8", "footprint": "SOIC-8-MAXIM-S8"}],
}

pcf8563 = {
 "id": "PCF8563", "function": "RTC",
 "description": "I2C real-time clock and calendar with alarm, timer flag and clock output",
 "drawer": ["Interface"],
 "note": "PCF8563.pdf (NXP): p.5 ordering information (PCF8563T/5, SO8, SOT96-1; ,518 is the reel), p.8 pinning (PCF8563T: 1 OSCI, 2 OSCO, 3 INT, 4 VSS, 5 SDA, 6 SCL, 7 CLKOUT, 8 VDD), register overview (00h-0Fh) and register descriptions, I2C slave address A2h write / A3h read (7-bit 51h) with an auto-incrementing register address, p.31 static characteristics (VDD 1.8-5.5 V with the interface active). Reset values: Control_status_1 08h (TESTC), VL = 1, CLKOUT_control 80h, Timer_control 03h; the datasheet leaves the alarm registers undefined, they start with their AE bits 1 (disabled). Modelled: time and calendar (weekday 0 = Sunday), the century bit, STOP, the alarm (minute, hour, day, weekday; an AE bit of 1 takes that entry out of the comparison) on INT (open drain). Left out: CLKOUT, the countdown timer's counting and its flag, the low-voltage detector (VL only reads its reset value). With all four AE bits 1 the chip raises no alarm, but the engine treats four masked entries as a match, so AF sets every second then (INT stays released while AIE = 0).",
 "pinouts": {"so8": pins(("OSCI", "input"), ("OSCO", "output"), ("INT", "output", 1), ("VSS", "ground"), ("SDA", "inout"), ("SCL", "input"), ("CLKOUT", "output"), ("VDD", "power"))},
 "vcc": [1.8, 5.5],
 "analog": ["OSCI", "OSCO"],
 "bus": {"style": "i2c", "scl": "SCL", "sda": "SDA", "address": [], "table": [81]},
 "registers": [
  reg("CONTROL_STATUS_1", 0, 0x08, [("TEST1", 7, 7), ("N6", 6, 6), ("STOP", 5, 5), ("N4", 4, 4), ("TESTC", 3, 3), ("N", 2, 0)]),
  reg("CONTROL_STATUS_2", 1, 0x00, [("N", 7, 5), ("TI_TP", 4, 4), ("AF", 3, 3), ("TF", 2, 2), ("AIE", 1, 1), ("TIE", 0, 0)]),
  reg("VL_SECONDS", 2, 0x80, [("VL", 7, 7), ("SECONDS", 6, 0)]),
  reg("MINUTES", 3, 0, [("X", 7, 7), ("MINUTES", 6, 0)]),
  reg("HOURS", 4, 0, [("X", 7, 6), ("HOURS", 5, 0)]),
  reg("DAYS", 5, 1, [("X", 7, 6), ("DAYS", 5, 0)]),
  reg("WEEKDAYS", 6, 6, [("X", 7, 3), ("WEEKDAYS", 2, 0)]),
  reg("CENTURY_MONTHS", 7, 1, [("C", 7, 7), ("X", 6, 5), ("MONTHS", 4, 0)]),
  reg("YEARS", 8, 0, [("YEARS", 7, 0)]),
  reg("MINUTE_ALARM", 9, 0x80, [("AE_M", 7, 7), ("MINUTE_ALARM", 6, 0)]),
  reg("HOUR_ALARM", 10, 0x80, [("AE_H", 7, 7), ("X", 6, 6), ("HOUR_ALARM", 5, 0)]),
  reg("DAY_ALARM", 11, 0x80, [("AE_D", 7, 7), ("X", 6, 6), ("DAY_ALARM", 5, 0)]),
  reg("WEEKDAY_ALARM", 12, 0x80, [("AE_W", 7, 7), ("X", 6, 3), ("WEEKDAY_ALARM", 2, 0)]),
  reg("CLKOUT_CONTROL", 13, 0x80, [("FE", 7, 7), ("X", 6, 2), ("FD", 1, 0)]),
  reg("TIMER_CONTROL", 14, 0x03, [("TE", 7, 7), ("X", 6, 2), ("TD", 1, 0)]),
  reg("TIMER", 15, 0),
 ],
 "protocol": POINTER,
 "clock": {
  "fields": [
   {"field": "VL_SECONDS.SECONDS", "unit": "second", "format": "bcd"},
   {"field": "MINUTES.MINUTES", "unit": "minute", "format": "bcd"},
   {"field": "HOURS.HOURS", "unit": "hour", "format": "bcd"},
   {"field": "WEEKDAYS.WEEKDAYS", "unit": "weekday", "first": 0},
   {"field": "DAYS.DAYS", "unit": "date", "format": "bcd"},
   {"field": "CENTURY_MONTHS.MONTHS", "unit": "month", "format": "bcd"},
   {"field": "YEARS.YEARS", "unit": "year", "format": "bcd", "base": 2000},
   {"field": "CENTURY_MONTHS.C", "unit": "century", "format": "binary"}],
  "stop": {"field": "CONTROL_STATUS_1.STOP", "stopped": 1, "powerOn": 0},
  "update": "none",
  "alarms": [
   {"match": [{"field": "MINUTE_ALARM.MINUTE_ALARM", "unit": "minute", "mask": "MINUTE_ALARM.AE_M"},
              {"field": "HOUR_ALARM.HOUR_ALARM", "unit": "hour", "mask": "HOUR_ALARM.AE_H"},
              {"field": "DAY_ALARM.DAY_ALARM", "unit": "date", "mask": "DAY_ALARM.AE_D"},
              {"field": "WEEKDAY_ALARM.WEEKDAY_ALARM", "unit": "weekday", "mask": "WEEKDAY_ALARM.AE_W"}],
    "flag": "CONTROL_STATUS_2.AF", "enable": "CONTROL_STATUS_2.AIE", "pin": {"pin": "INT", "active": 0, "openDrain": True}}]},
 "parts": [{"name": "PCF8563T/5,518", "manufacturer": "NXP", "datasheet": "pcf8563", "package": "SOIC", "pinout": "so8", "footprint": "SOIC-8-NXP-SOT96-1"}],
}

spec = {
 "datasheets": [{"id": "ds3231m", "url": "https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231M.pdf"},
                {"id": "pcf8563", "url": "https://www.nxp.com/docs/en/data-sheet/PCF8563.pdf"}],
 "footprints": [
  {"id": "SOIC-8-MAXIM-S8", "description": "8-pin SOIC (Maxim S8, 0.150 in), 3.9 x 4.9 mm body, 1.27 mm pitch",
   "note": "Maxim/ADI 90-0096 rev M land pattern [S8, incl. S8MK+1] p.1 (R1; 1.98 x 0.53 pads, 1.27 pitch, 4.93 pad-centre span), analog.com/media/en/package-pcb-resources/land-pattern/soicn/90-0096.pdf; body E 3.90 typ (3.80-4.00) x D 4.90 typ (4.80-5.00) from outline 21-0041 rev V, analog.com/media/en/package-pcb-resources/package/pkg_pdf/soicn/21-0041.pdf; DS3231M code S8MK+1 named in rtc/DS3231M.pdf p.19. Not SOIC-8 (TI: 1.55 x 0.6 pads, span 5.4)",
   "style": "dual", "pins": 8, "body": {"width": 3.9, "height": 4.9}, "pitch": 1.27, "span": 4.93, "pad": {"width": 1.98, "height": 0.53, "shape": "rect"}},
  {"id": "SOIC-8-NXP-SOT96-1", "description": "8-pin SO8 (NXP SOT96-1), 3.9 x 4.9 mm body, 1.27 mm pitch",
   "note": "NXP SOT96-1 package outline (R2; no land pattern printed), datasheets/rtc/PCF8563.pdf p.38 Figure 30: Lp 0.4-1.0 -> pad length 1.80; bp 0.36-0.49 -> pad width 0.54; HE 5.8-6.2 -> span 6.2 + 1.0 - 1.80 = 5.40; e 1.27; body E 3.90 mid (3.8-4.0) x D 4.90 mid (4.8-5.0). Not SOIC-8 (TI: 1.55 x 0.6 pads)",
   "style": "dual", "pins": 8, "body": {"width": 3.9, "height": 4.9}, "pitch": 1.27, "span": 5.4, "pad": {"width": 1.8, "height": 0.54, "shape": "rect"}}],
 "devices": {"rtc": [ds3231m, pcf8563]}}
json.dump(spec, open('planning/plan-43-data/add/rtc.json', 'w'), indent=1, ensure_ascii=False)
print('ok')
