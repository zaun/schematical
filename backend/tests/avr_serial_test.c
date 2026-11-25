/*
 * avr_serial_test (plan-62-F): the AVR's USART, SPI master and TWI master on
 * their pins (src/models/mcu_avr_serial.c), against library parts that speak
 * the same buses. avr8js stops at the byte, so it cannot check this layer;
 * each case asserts on the partner part's own state.
 *
 *   USART  TESTAVR with an MCP2200-I/SO USB-to-UART bridge (TXD to its Rx,
 *          its Tx to RXD): the bridge receives "HI", a typed byte comes
 *          back + 1, the bit time is 104 us at 9600 baud on the 1 MHz
 *          clock, and a 7E2 frame has its parity and stop bits.
 *   SPI    TESTAVR with an AT25320B-SSHL-B SPI EEPROM: three bytes written
 *          and read back in mode 0; mode 3 idles SCK high; with the DDR
 *          bits clear nothing is driven.
 *   TWI    TESTAVR with a PCF8563T/5,518 I2C real-time clock: a register
 *          written and read back, the start and stop conditions and 9
 *          clocks per byte, an address nobody answers, and the same on the
 *          ATmega328PB's second TWI (fixture TESTAVR328PB).
 *
 * Firmware: tests/avr/<name>.hex, from tests/avr/src/<name>.asm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "converter_state.h"
#include "mcu_fixture.h"
#include "parts.h"
#include "read_file.h"

static PartLibrary *library;
static int failures;

#include "avr_rig.h"

static void check(int ok, const char *name) {
    printf("%s %s\n", ok ? "ok" : "FAIL:", name);
    if (!ok) {
        failures++;
    }
}

static size_t add_part(Rig *r, const char *name) {
    size_t index = 0;

    if (circuit_add_part(r->c, name, &index) != 0) {
        printf("FAIL: %s: %s\n", name, circuit_error(r->c));
        failures++;
    }
    return index;
}

/* A port's eight pins as a number; -1 when one is not 0 or 1. */
static int port_value(Rig *r, char letter) {
    int value = 0, bit;
    char name[4] = {'P', letter, '0', '\0'};

    for (bit = 0; bit < 8; bit++) {
        CircuitValue v;

        name[2] = (char)('0' + bit);
        v = circuit_net_value(r->c, rig_net(r, name));
        if (v != CIRCUIT_0 && v != CIRCUIT_1) {
            return -1;
        }
        value |= (v == CIRCUIT_1) << bit;
    }
    return value;
}

/* ---- USART ------------------------------------------------------------------ */

/* The bridge next to the chip: TXD (PD1) to its Rx, its Tx to RXD (PD0). */
static size_t add_bridge(Rig *r) {
    size_t bridge = add_part(r, "MCP2200-I/SO");

    circuit_connect(r->c, bridge, "1", r->n_vcc);    /* VDD */
    circuit_connect(r->c, bridge, "4", r->n_vcc);    /* RST, not asserted */
    circuit_connect(r->c, bridge, "20", r->n_gnd);   /* VSS */
    circuit_connect(r->c, bridge, "12", rig_net(r, "PD1"));   /* Rx */
    circuit_connect(r->c, bridge, "10", rig_net(r, "PD0"));   /* Tx */
    return bridge;
}

static void usart_cases(void) {
    Rig r;
    size_t bridge;
    uint16_t frames[8] = {0};
    uint64_t t[12];
    size_t txd;
    int i, ok;

    /* "HI", then the echo. */
    rig_new(&r, "TESTAVR", "usart_hi");
    bridge = add_bridge(&r);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 6 * MS);
    check(circuit_serial_sent_count(r.c, bridge, 0) == 2 &&
              circuit_serial_sent(r.c, bridge, 0, 0, frames, 8) == 2 && frames[0] == 'H' &&
              frames[1] == 'I',
          "usart_hi (the bridge received \"HI\")");
    check(circuit_serial_send(r.c, bridge, 0, (const uint8_t *)"a", 1) == 0, "usart_type (a byte typed into the bridge)");
    circuit_advance(r.c, 6 * MS);
    check(circuit_serial_sent(r.c, bridge, 0, 0, frames, 8) == 3 && frames[2] == 'b',
          "usart_echo (the chip received 'a' and sent 'b')");
    circuit_destroy(r.c);

    /* The waveform of 'H' (0x48): start, 0 0 0 1 0 0 1 0, stop; 104 us per bit. */
    rig_new(&r, "TESTAVR", "usart_hi");
    txd = rig_net(&r, "PD1");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    ok = 1;
    for (i = 0; i < 7; i++) {
        t[i] = next_change(&r, txd, US, 5 * MS);
        ok &= t[i] != 0;
    }
    /* t[0]: Z to 1 (TXEN); t[1]: the start bit; then the edges of the frame. */
    check(ok && t[2] - t[1] == 4 * 104 * US && t[3] - t[2] == 104 * US && t[4] - t[3] == 2 * 104 * US &&
              t[5] - t[4] == 104 * US && t[6] - t[5] == 104 * US &&
              circuit_net_value(r.c, txd) == CIRCUIT_1,
          "usart_bit_time (TXD idles at 1; 'H' has its edges 104 us x 4, 1, 2, 1, 1 apart)");
    circuit_destroy(r.c);

    /* 7E2: 0x55 is start, 1 0 1 0 1 0 1, parity 0, stop, stop. */
    rig_new(&r, "TESTAVR", "usart_7e2");
    bridge = add_bridge(&r);
    txd = rig_net(&r, "PD1");
    circuit_serial_configure(r.c, bridge, 0, 9600, 7, 2, 4, 0);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    ok = 1;
    for (i = 0; i < 11; i++) {
        t[i] = next_change(&r, txd, US, 5 * MS);
        ok &= t[i] != 0;
    }
    for (i = 2; i < 11; i++) {
        ok &= t[i] - t[i - 1] == 104 * US;   /* 9 edges one bit apart: the last rises into the stop bits */
    }
    check(ok && next_change(&r, txd, US, 3 * MS) == 0 && circuit_net_value(r.c, txd) == CIRCUIT_1 &&
              circuit_serial_sent(r.c, bridge, 0, 0, frames, 8) == 1 && frames[0] == 0x55,
          "usart_7e2 (7 data bits, an even parity bit of 0, then high; the bridge set to 7E2 received 0x55)");
    circuit_destroy(r.c);
}

/* ---- SPI ---------------------------------------------------------------------- */

/* The EEPROM: CS# on PB2, SCK on PB5, SI on PB3 (MOSI), SO on PB4 (MISO). */
static size_t add_eeprom(Rig *r) {
    size_t eeprom = add_part(r, "AT25320B-SSHL-B");

    circuit_connect(r->c, eeprom, "1", rig_net(r, "PB2"));   /* CS# */
    circuit_connect(r->c, eeprom, "2", rig_net(r, "PB4"));   /* SO */
    circuit_connect(r->c, eeprom, "3", r->n_vcc);            /* WP# */
    circuit_connect(r->c, eeprom, "4", r->n_gnd);
    circuit_connect(r->c, eeprom, "5", rig_net(r, "PB3"));   /* SI */
    circuit_connect(r->c, eeprom, "6", rig_net(r, "PB5"));   /* SCK */
    circuit_connect(r->c, eeprom, "7", r->n_vcc);            /* HOLD# */
    circuit_connect(r->c, eeprom, "8", r->n_vcc);
    return eeprom;
}

static void spi_cases(void) {
    Rig r;
    size_t eeprom, sck;
    uint32_t values[3] = {0}, known[3] = {0};
    uint64_t changes;

    rig_new(&r, "TESTAVR", "spi_eeprom");
    eeprom = add_eeprom(&r);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 20 * MS);
    check(circuit_memory_read(r.c, eeprom, 0x10, 3, values, known) == 3 && values[0] == 0xa1 &&
              values[1] == 0xb2 && values[2] == 0xc3 && port_value(&r, 'D') == 0xc3 &&
              circuit_error(r.c) == NULL,
          "spi_eeprom (A1 B2 C3 written at 0x0010 of the EEPROM, and the last byte read back on PORTD)");
    if (port_value(&r, 'D') != 0xc3) {
        printf("     the EEPROM holds %02x %02x %02x, PORTD %02x\n", (unsigned)values[0],
               (unsigned)values[1], (unsigned)values[2], (unsigned)port_value(&r, 'D'));
    }
    circuit_destroy(r.c);

    /* Mode 3: SCK goes Z, 0 (the port), 1 (idle), then 16 edges for each of 3 bytes. */
    rig_new(&r, "TESTAVR", "spi_mode3");
    eeprom = add_eeprom(&r);
    sck = rig_net(&r, "PB5");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 10 * MS);
    changes = circuit_net_changes(r.c, sck);
    check(port_value(&r, 'D') == 0x02 && circuit_net_value(r.c, sck) == CIRCUIT_1 && changes == 2 + 3 * 16,
          "spi_mode3 (SCK idles high, 16 edges per byte; the EEPROM answered RDSR with WEL set)");
    if (changes != 2 + 3 * 16) {
        printf("     SCK changed %llu times, PORTD %02x\n", (unsigned long long)changes,
               (unsigned)port_value(&r, 'D'));
    }
    circuit_destroy(r.c);

    /* The DDR bits clear: the master drives neither SCK nor MOSI. */
    rig_new(&r, "TESTAVR", "spi_noddr");
    sck = rig_net(&r, "PB5");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 5 * MS);
    check(port_value(&r, 'D') == 0x5a && circuit_net_changes(r.c, sck) == 0 &&
              circuit_net_value(r.c, sck) == CIRCUIT_Z &&
              circuit_net_value(r.c, rig_net(&r, "PB3")) == CIRCUIT_Z,
          "spi_ddr (with their DDR bits clear SCK and MOSI stay Z, and the transfer still completes)");
    circuit_destroy(r.c);
}

/* ---- TWI ------------------------------------------------------------------------ */

static size_t add_rtc(Rig *r, const char *sda, const char *scl) {
    size_t rtc = add_part(r, "PCF8563T/5,518");

    circuit_connect(r->c, rtc, "4", r->n_gnd);              /* VSS */
    circuit_connect(r->c, rtc, "5", rig_net(r, sda));       /* SDA */
    circuit_connect(r->c, rtc, "6", rig_net(r, scl));       /* SCL */
    circuit_connect(r->c, rtc, "8", r->n_vcc);              /* VDD */
    return rtc;
}

/* The RTC's register at a bus address, or -1. */
static int rtc_register(Rig *r, size_t rtc, int address) {
    ConverterRegister registers[64];
    int count = circuit_converter_registers(r->c, rtc, registers, 64);
    int i;

    for (i = 0; i < count && i < 64; i++) {
        if (registers[i].address == address) {
            return (int)registers[i].value;
        }
    }
    return -1;
}

static void twi_cases(void) {
    Rig r;
    size_t rtc, sda, scl;
    uint64_t scl_before, start_time;

    rig_new(&r, "TESTAVR", "twi_rtc");
    rtc = add_rtc(&r, "PC4", "PC5");
    sda = rig_net(&r, "PC4");
    scl = rig_net(&r, "PC5");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    /* The pull-ups come on (Z to 1) before the start condition. */
    while (circuit_net_value(r.c, sda) != CIRCUIT_1 && circuit_time(r.c) < 2 * MS) {
        circuit_advance(r.c, US);
    }
    scl_before = circuit_net_changes(r.c, scl);
    start_time = next_change(&r, sda, US, 5 * MS);
    check(start_time != 0 && circuit_net_value(r.c, sda) == CIRCUIT_0 &&
              circuit_net_value(r.c, scl) == CIRCUIT_1,
          "twi_start (SDA falls while SCL is high)");
    rig_to(&r, 20 * MS);
    check(port_value(&r, 'B') == 0x25 && (rtc_register(&r, rtc, 2) & 0x7f) == 0x25 &&
              circuit_error(r.c) == NULL,
          "twi_rtc (0x25 written to the clock's seconds register and read back on PORTB)");
    if (port_value(&r, 'B') != 0x25) {
        printf("     PORTB %02x, the register %02x\n", (unsigned)port_value(&r, 'B'),
               (unsigned)rtc_register(&r, rtc, 2));
    }
    /* SCL: 1 fall for the start, 2 changes for each of 7 bytes x 9 clocks, 2
       for each of the 2 repeated starts, 1 rise for the stop. */
    check(circuit_net_changes(r.c, scl) - scl_before == 1 + 7 * 9 * 2 + 2 * 2 + 1 &&
              circuit_net_value(r.c, sda) == CIRCUIT_1 && circuit_net_value(r.c, scl) == CIRCUIT_1,
          "twi_waveform (9 clocks per byte, and the bus is released after the stop)");
    if (circuit_net_changes(r.c, scl) - scl_before != 132) {
        printf("     SCL changed %llu times\n", (unsigned long long)(circuit_net_changes(r.c, scl) - scl_before));
    }
    circuit_destroy(r.c);

    rig_new(&r, "TESTAVR", "twi_nack");
    rtc = add_rtc(&r, "PC4", "PC5");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 10 * MS);
    check(port_value(&r, 'B') == 0x20, "twi_nack (address 0x20: status 0x20, no acknowledge)");
    circuit_destroy(r.c);

    /* The ATmega328PB's TWI1 on PE0 and PE1. */
    rig_new(&r, "TESTAVR328PB", "twi1_rtc");
    rtc = add_rtc(&r, "PE0", "PE1");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 20 * MS);
    check(port_value(&r, 'D') == 0x25 && (rtc_register(&r, rtc, 2) & 0x7f) == 0x25,
          "twi1_rtc (the same through TWI1 of an ATmega328PB)");
    if (port_value(&r, 'D') != 0x25) {
        printf("     PORTD %02x, the register %02x\n", (unsigned)port_value(&r, 'D'),
               (unsigned)rtc_register(&r, rtc, 2));
    }
    circuit_destroy(r.c);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0 || load_mcu_fixture(library) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    usart_cases();
    spi_cases();
    twi_cases();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("avr_serial_test: all passed\n");
    return 0;
}
