"""plan-68-J: the eleven backend files, by plan-68-J § 3."""
import json, subprocess, sys
sys.path.insert(0, '/private/tmp/claude-501/p68')
import split_model

M = 'models/'
SPECS = [
    (M + 'converter_load.c', M + 'converter_load_int.h', [
        (('sec', '---- Registers'), M + 'converter_load_proto.c'),
        (('sec', '---- Conversion, ready, pins'), M + 'converter_load_conv.c'),
        (('sec', "---- The rtc kind's clock"), M + 'converter_load_rtc.c'),
        (('sec', '---- The display section'), M + 'converter_load_display.c'),
        (('fn', 'lcd_mode_slots'), M + 'converter_load_lcd.c'),
        (('sec', '---- The device'), M + 'converter_load.c'),
    ], 'cvl_'),
    (M + 'converter.c', M + 'converter_core_int.h', [
        (('sec', '---- The protocol engine: bits in'), M + 'converter_proto.c'),
        (('sec', '---- The model'), M + 'converter_model.c'),
        (('sec', '---- The kind'), M + 'converter.c'),
    ], 'cvc_'),
    ('library/library.c', 'library/library_load_int.h', [
        (('fn', 'file_add_part'), 'library/library_load.c'),
        (('fn', 'load_device'), 'library/library_load_device.c'),
        (('sec', '---- Single part files'), 'library/library.c'),
    ], 'lib_'),
    (M + 'serialmem.c', M + 'serialmem_int.h', [
        (('sec', '---- Commands'), M + 'serialmem_cmd.c'),
        (('sec', '---- Pins'), M + 'serialmem.c'),
        (('sec', '---- Loader'), M + 'serialmem_load.c'),
    ], 'smem_'),
    (M + 'counter.c', M + 'counter_int.h', [
        (('sec', '---- sync ----'), M + 'counter_sync.c'),
        (('sec', '---- sync: loading'), M + 'counter_sync_load.c'),
        (('sec', '---- sync: counting'), M + 'counter_sync.c'),
        (('sec', '---- ripple'), M + 'counter_ripple.c'),
        (('sec', '---- The kind'), M + 'counter.c'),
    ], 'cnt_'),
    (M + 'counter_div.c', M + 'counter_div_int.h', [
        (('sec', '---- seg7'), M + 'counter_seg7.c'),
        (('sec', '---- divn'), M + 'counter_div.c'),
    ], 'cdv_'),
    (M + 'shift.c', M + 'shift_int.h', [
        (('sec', '---- loading ----'), M + 'shift_load.c'),
        (('sec', '---- running ----'), M + 'shift.c'),
    ], 'shf_'),
    (M + 'pio_via6522.c', M + 'pio_via6522_int.h', [
        (('sec', '---- Reads'), M + 'pio_via6522_regs.c'),
        (('sec', '---- Cycle hooks'), M + 'pio_via6522.c'),
    ], 'v22_'),
    ('engine/sim.c', 'engine/sim_int.h', [
        (('sec', '---- Contacts'), 'engine/sim_contacts.c'),
    ], 'simc_'),
    (M + 'charlcd.c', M + 'charlcd_int.h', [
        (('sec', '---- Loader'), M + 'charlcd_load.c'),
        (('sec', '---- Model'), M + 'charlcd.c'),
    ], 'clcd_'),
    (M + 'eeprom.c', M + 'eeprom_int.h', [
        (('sec', '---- Kind: devices, pinouts, parts'), M + 'eeprom_kind.c'),
    ], 'eep_'),
]
only = set(sys.argv[1:])
made = {}
for path, header, cuts, prefix in SPECS:
    if only and path not in only:
        continue
    made[path] = split_model.split(path, header, cuts, prefix)
print(json.dumps(made))
