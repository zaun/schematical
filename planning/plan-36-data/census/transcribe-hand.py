#!/usr/bin/env python3
"""plan-36-B Phase 2: writes transcribed-hand.tsv: the figures transcribe.py cannot read,
typed from the rendered pages (pdftoppm -r 90-110), plus readings used only to check the auto ones."""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import roles as R
M = 'datasheets/mcu/microchip/'
H = [
    # pdf, page, figure, devices, pinout, names in pin order
    (M + 'Atmel-4317-8-bit-AVR-Flash_Microcontroller-AT90PWM2-3-2B-3B_summary.pdf', 5,
     'Table 3-1 Pin out description, QFN32 Pin Number column (pp.5-6)', 'AT90PWM3B', 'quad32',
     'PD2 PD3 PC1 VCC GND PC2 PC3 PB0 PB1 PE1 PE2 PD4 PD5 PD6 PD7 PB2 PC4 PC5 AVCC AGND AREF PC6 PB3 PB4 PC7 PB5 PB6 PB7 PD0 PC0 PE0 PD1'),
    (M + 'Atmel-7710-8-bit-Microcontroller-AT90PWM216-316_Datasheet-Summary.pdf', 5,
     'Table Pin out description, QFN32 Pin column (Figure 2-3 QFN32 7x7)', 'AT90PWM316', 'quad32',
     'PD2 PD3 PC1 VCC GND PC2 PC3 PB0 PB1 PE1 PE2 PD4 PD5 PD6 PD7 PB2 PC4 PC5 AVCC AGND AREF PC6 PB3 PB4 PC7 PB5 PB6 PB7 PD0 PC0 PE0 PD1'),
    (M + 'Atmel-8303-8-bit-AVR-Microcontroller-tinyAVR-ATtiny1634_Datasheet_summary.pdf', 2,
     'Figure 1-1 Pinout of ATtiny1634, QFN/MLF', 'ATTINY1634,ATTINY1634R', 'quad20',
     'PA6 PA5 PA4 PA3 PA2 PA1 PA0 GND VCC PC5 PC4 PC3 PC2 PC1 PC0 PB3 PB2 PB1 PB0 PA7'),
    (M + 'doc2502.pdf', 2, 'Figure 1 Pinout ATmega8535, PDIP', 'ATMEGA8535', 'p40',
     'PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7 RESET VCC GND XTAL2 XTAL1 PD0 PD1 PD2 PD3 PD4 PD5 PD6 PD7 '
     'PC0 PC1 PC2 PC3 PC4 PC5 PC6 PC7 AVCC GND AREF PA7 PA6 PA5 PA4 PA3 PA2 PA1 PA0'),
    (M + 'doc7734.pdf', 4, 'Figure 2-2 32-pin packages, AT90PWM81/161 QFN 32 5*5', 'AT90PWM161,AT90PWM81', 'quad32',
     'NC PD0 PB1 PB2 VCC GND PE1 NC NC PE2 PD1 PD2 PD3 PB3 PB4 NC NC PD4 PB5 AVCC AGND PE3 PD5 NC NC PD6 PB6 PD7 PB7 PB0 PE0 NC'),
]
BALLS = [
    (M + '8183s.pdf', 2, 'Table 1-1 UFBGA - Pinout ATtiny24A/44A/84A (top view); A1 has no ball', 'ATTINY24A,ATTINY44A,ATTINY84A', 'bga15',
     [('A2', 'PA5'), ('A3', 'PA6'), ('A4', 'PB2'), ('B1', 'PA4'), ('B2', 'PA7'), ('B3', 'PB1'), ('B4', 'PB3'),
      ('C1', 'PA3'), ('C2', 'PA2'), ('C3', 'PA1'), ('C4', 'PB0'), ('D1', 'PA0'), ('D2', 'GND'), ('D3', 'GND'), ('D4', 'VCC')]),
    (M + 'ATtiny24A-44A-84A-DataSheet-DS40002269A.pdf', 3, 'Table 1-1 UFBGA - Pinout ATtiny24A/44A/84A (top view); A1 has no ball',
     'ATTINY24A,ATTINY44A,ATTINY84A', 'bga15',
     [('A2', 'PA5'), ('A3', 'PA6'), ('A4', 'PB2'), ('B1', 'PA4'), ('B2', 'PA7'), ('B3', 'PB1'), ('B4', 'PB3'),
      ('C1', 'PA3'), ('C2', 'PA2'), ('C3', 'PA1'), ('C4', 'PB0'), ('D1', 'PA0'), ('D2', 'GND'), ('D3', 'GND'), ('D4', 'VCC')]),
]
RP = ('IOVDD GPIO0 GPIO1 GPIO2 GPIO3 GPIO4 GPIO5 GPIO6 GPIO7 IOVDD GPIO8 GPIO9 GPIO10 GPIO11 '
      'GPIO12 GPIO13 GPIO14 GPIO15 TESTEN XIN XOUT IOVDD DVDD SWCLK SWDIO RUN GPIO16 GPIO17 '
      'GPIO18 GPIO19 GPIO20 GPIO21 IOVDD GPIO22 GPIO23 GPIO24 GPIO25 GPIO26 GPIO27 GPIO28 GPIO29 IOVDD '
      'ADC_AVDD VREG_VIN VREG_VOUT USB_DM USB_DP USB_VDD IOVDD DVDD QSPI_SD3 QSPI_SCLK QSPI_SD0 QSPI_SD2 QSPI_SD1 QSPI_SS_N GND')
CHECK = [
    (M + '1477S.pdf', 2, 'check', 'ATTINY26', 'p20',
     'PB0 PB1 PB2 PB3 VCC GND PB4 PB5 PB6 PB7 PA7 PA6 PA5 PA4 AVCC GND PA3 PA2 PA1 PA0'),
    (M + '1477S.pdf', 2, 'check', 'ATTINY26', 'quad32',
     'NC PB3 NC VCC GND NC PB4 PB5 NC PB6 PB7 NC PA7 PA6 PA5 NC PA4 AVCC NC NC GND PA3 PA2 NC PA1 PA0 NC NC NC PB0 PB1 PB2'),
    (M + 'Atmel-2548-8-bit-AVR-Microcontroller-Battery-Management-ATmega406_Datasheet-Summary.pdf', 2, 'check', 'ATMEGA406', 'quad48',
     'SGND PA0 PA1 PA2 PA3 VREG VCC GND PA4 PA5 PA6 PA7 RESET XTAL1 XTAL2 GND PB0 PB1 PB2 PB3 PB4 PB5 SCL SDA '
     'PB6 PB7 PD0 PD1 GND PC0 BATT OPC OC VFET OD PVT GND PV4 PV3 PV2 PV1 NV VREF VREFGND PPI PI NI NNI'),
    (M + 'doc2512.pdf', 2, 'check', 'ATMEGA8515', 'p40',
     'PB0 PB1 PB2 PB3 PB4 PB5 PB6 PB7 RESET PD0 PD1 PD2 PD3 PD4 PD5 PD6 PD7 XTAL2 XTAL1 GND '
     'PC0 PC1 PC2 PC3 PC4 PC5 PC6 PC7 PE2 PE1 PE0 PA7 PA6 PA5 PA4 PA3 PA2 PA1 PA0 VCC'),
]
out = sys.argv[1]
with open(out, 'w') as f:
    f.write('pdf\tpage\tfigure\tdevices\tpinout\tdesignator\tname\trole\n')
    for pdf, page, fig, devs, po, names in H:
        for i, nm in enumerate(names.split(), 1):
            role = 'nc' if nm in ('NC', 'DNC') else R.role(nm, 'avr')
            assert role, nm
            f.write(f'{pdf}\t{page}\thand: {fig}\t{devs}\t{po}\t{i}\t{nm}\t{role}\n')
    # ball pinouts: (ball, name)
    for pdf, page, fig, devs, po, balls in BALLS:
        for ball, nm in balls:
            f.write(f'{pdf}\t{page}\thand: {fig}\t{devs}\t{po}\t{ball}\t{nm}\t{R.role(nm, "avr")}\n')
    # RP2040: pins 1-56 from Figure 3, plus the exposed pad, pin 57 GND (B § 3.3)
    for i, nm in enumerate(RP.split(), 1):
        role = R.role(nm, 'rp2040')
        assert role, nm
        f.write(f'datasheets/mcu/raspberry-pi/rp2040-datasheet.pdf\t12\thand: § 1.4.1 Figure 3 RP2040 Pinout for QFN-56 7x7mm, '
                f'pin 57 = the exposed GND pad\tRP2040\tquad57\t{i}\t{nm}\t{role}\n')
with open(out.replace('.tsv', '-checks.tsv'), 'w') as f:
    f.write('pdf\tdevices\tpinout\tnames\n')
    for pdf, page, fig, devs, po, names in CHECK:
        f.write(f'{pdf}\t{devs}\t{po}\t{names}\n')
print('ok')
