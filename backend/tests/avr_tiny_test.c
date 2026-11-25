/*
 * avr_tiny_test: the ATtiny25/45/85 on the engine (model "attiny85":
 * src/models/avr_chip.c's layout, avr_timer_tiny.c, avr_usi.c, and the
 * adapter mcu_avr.c), as shipped parts in circuits of library parts.
 *
 * avr_periph_test checks the chip's registers against avr8js. This test
 * covers what avr8js cannot: the pins (clock on PB3, reset on PB5, the ADC
 * inputs), the places where the simulator follows the datasheet instead of
 * avr8js (Timer1's PWM mode, the INT0 flag cleared through GIFR, PLOCK),
 * and the USI, which is not an avr8js port: as an SPI master against an SPI
 * EEPROM and as an I2C master against an I2C real-time clock.
 *
 * Firmware: tests/avr/<name>.hex, from tests/avr/src/<name>.asm.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "circuit.h"
#include "converter_state.h"
#include "parts.h"
#include "read_file.h"

static PartLibrary *library;
static int failures;

#include "avr_rig.h"

#define TINY85 "ATTINY85-20PU"
#define TINY25 "ATTINY25-20PU"

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

static int is(Rig *r, const char *pin, CircuitValue value) {
    return circuit_net_value(r->c, rig_net(r, pin)) == value;
}

/* ---- Facts, startup, the clocks, reset ---------------------------------------- */

static void fact_cases(void) {
    const Part *t85 = part_library_find(library, TINY85);
    const Part *t25 = part_library_find(library, TINY25);
    const Part *t45 = part_library_find(library, "ATTINY45-20PU");
    Rig r;
    uint32_t words = 0;
    unsigned bits = 0;

    check(t85 != NULL && t85->simulated == 1 && t85->mcu != NULL && t85->mcu->model != NULL &&
              strcmp(t85->mcu->model, "attiny85") == 0 && t85->pin_count == 8,
          "facts (ATTINY85-20PU is simulated, model attiny85, 8 pins)");
    check(t25 != NULL && t25->simulated == 1 && t45 != NULL && t45->simulated == 1 &&
              strcmp(t25->mcu->model, "attiny85") == 0 && strcmp(t45->mcu->model, "attiny85") == 0,
          "facts (the ATtiny25 and ATtiny45 use the same model)");
    rig_new(&r, TINY85, NULL);
    check(circuit_memory_size(r.c, r.avr, &words, &bits) == 1 && words == 8192 && bits == 8,
          "memory (an ATtiny85's flash is 8192 bytes)");
    circuit_destroy(r.c);
    rig_new(&r, TINY25, NULL);
    check(circuit_memory_size(r.c, r.avr, &words, &bits) == 1 && words == 2048 && bits == 8,
          "memory (an ATtiny25's is 2048)");
    circuit_destroy(r.c);
}

static void blink_cases(void) {
    Rig r;
    uint64_t first, period;
    CpuState state;

    rig_new(&r, TINY85, "t85_blink");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS / 2);
    check(is(&r, "PB0", CIRCUIT_Z) && is(&r, "PB1", CIRCUIT_Z) && is(&r, "PB2", CIRCUIT_Z) &&
              is(&r, "PB3", CIRCUIT_Z) && is(&r, "PB4", CIRCUIT_Z) && is(&r, "PB5", CIRCUIT_1),
          "startup (every port pin is Z; the reset pin PB5 reads 1, its pull-up)");
    rig_to(&r, MS);
    first = next_change(&r, rig_net(&r, "PB0"), US, 10 * MS);
    period = toggle_period(&r, rig_net(&r, "PB0"), US, 10 * MS, US);
    check(first > MS && period == 1000 * US,
          "blink (internal clock: PB0 changes every 1000 cycles of 1 us, by the Timer1 compare interrupt)");
    if (period != 1000 * US) {
        printf("     period %llu ps\n", (unsigned long long)period);
    }
    check(circuit_cpu_state(r.c, r.avr, &state) == 0 && state.status == CPU_RUNNING &&
              state.register_count == 35 && strcmp(state.registers[1].name, "SP") == 0 &&
              state.registers[1].value >= 0x25b && state.registers[1].value <= 0x25f,
          "state (35 registers; SP is at the top of the SRAM, 0x025F)");
    check(is(&r, "PB3", CIRCUIT_Z) && is(&r, "PB4", CIRCUIT_Z),
          "clock pins (on the internal clock PB3 and PB4 are port pins: inputs, Z)");
    circuit_destroy(r.c);

    /* The same program on an ATtiny25: 128 bytes of SRAM, the stack at 0xDF. */
    rig_new(&r, TINY25, "t85_blink");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    next_change(&r, rig_net(&r, "PB0"), US, 10 * MS);
    period = toggle_period(&r, rig_net(&r, "PB0"), US, 10 * MS, US);
    check(period == 1000 * US && circuit_cpu_state(r.c, r.avr, &state) == 0 &&
              state.registers[1].value >= 0xdb && state.registers[1].value <= 0xdf &&
              circuit_error(r.c) == NULL,
          "blink25 (an ATtiny25 runs it too, its stack at 0x00DF)");
    circuit_destroy(r.c);
}

static void crystal_case(void) {
    Rig r;
    uint64_t period, changes;

    rig_new(&r, TINY85, "t85_blink");
    rig_two_pin(&r, "GENERIC-CRYSTAL-16M", rig_net(&r, "PB3"), rig_net(&r, "PB4"));
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS);
    period = toggle_period(&r, rig_net(&r, "PB0"), 62500, 2 * MS, 62500);
    check(period == 62500000ull,
          "crystal (16 MHz on PB3/PB4: PB0 changes every 1000 cycles of 62.5 ns)");
    if (period != 62500000ull) {
        printf("     period %llu ps\n", (unsigned long long)period);
    }
    changes = circuit_net_changes(r.c, rig_net(&r, "PB3"));
    circuit_advance(r.c, 10 * US);
    check(circuit_net_changes(r.c, rig_net(&r, "PB3")) - changes == 320 && circuit_error(r.c) == NULL,
          "crystal (it keeps clocking PB3: the chip does not drive its clock pins)");
    circuit_destroy(r.c);
}

static void reset_case(void) {
    Rig r;
    size_t sw;
    uint64_t released, again;
    CpuState state;

    rig_new(&r, TINY85, "t85_blink");
    sw = rig_two_pin(&r, "AS11AH", rig_net(&r, "PB5"), r.n_gnd);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 3 * MS);
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, 10 * US);
    check(is(&r, "PB5", CIRCUIT_0) && is(&r, "PB0", CIRCUIT_Z) &&
              circuit_cpu_state(r.c, r.avr, &state) == 0 && state.status == CPU_RESET,
          "reset (PB5 pulled to GND: PB0 is released and the chip is in reset)");
    circuit_advance(r.c, 5 * MS);
    check(is(&r, "PB0", CIRCUIT_Z), "reset (nothing runs while it is held)");
    circuit_set_switch(r.c, sw, 1, 0);
    released = circuit_time(r.c);
    next_change(&r, rig_net(&r, "PB0"), US, 10 * MS);            /* Z to 0: DDRB0 set */
    again = next_change(&r, rig_net(&r, "PB0"), US, 10 * MS);    /* the first toggle */
    check(again != 0 && again - released < 3 * MS, "reset (released, the program starts over)");
    circuit_destroy(r.c);
}

/* ---- Where the simulator follows the datasheet, not avr8js ------------------------ */

/* The lengths of the next high and low times of a net, from a rising edge. */
static int high_low(Rig *r, size_t net, uint64_t *high, uint64_t *low) {
    uint64_t t[3];
    int i;

    do {
        t[0] = next_change(r, net, US, 10 * MS);
    } while (t[0] != 0 && circuit_net_value(r->c, net) != CIRCUIT_1);
    for (i = 1; i < 3; i++) {
        t[i] = next_change(r, net, US, 10 * MS);
    }
    *high = t[1] - t[0];
    *low = t[2] - t[1];
    return t[0] != 0 && t[1] != 0 && t[2] != 0;
}

static void pwm_case(void) {
    Rig r;
    uint64_t high = 0, low = 0, high_b = 0, low_b = 0, high_n = 0, low_n = 0;
    int ok;

    rig_new(&r, TINY85, "t85_pwm");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 2 * MS);
    check(circuit_net_changes(r.c, rig_net(&r, "PB1")) > 2,
          "pll (the program got past its wait for PLOCK)");
    ok = high_low(&r, rig_net(&r, "PB1"), &high, &low);
    check(ok && high == 26 * US && low == 74 * US,
          "pwm (OC1A on PB1: high 26 of every 100 cycles, TOP OCR1C = 99: one slope, not two)");
    ok = high_low(&r, rig_net(&r, "PB4"), &high_b, &low_b);
    check(ok && high_b == 60 * US && low_b == 40 * US, "pwm (OC1B on PB4: high 60 of every 100)");
    ok = high_low(&r, rig_net(&r, "PB0"), &high_n, &low_n);
    check(ok && high_n == 74 * US && low_n == 26 * US,
          "pwm (/OC1A on PB0, COM1A = 01: the inverse, high 74 of every 100)");
    if (high != 26 * US || high_b != 60 * US || high_n != 74 * US) {
        printf("     PB1 %llu/%llu us, PB4 %llu/%llu us, PB0 %llu/%llu us\n",
               (unsigned long long)(high / US), (unsigned long long)(low / US),
               (unsigned long long)(high_b / US), (unsigned long long)(low_b / US),
               (unsigned long long)(high_n / US), (unsigned long long)(low_n / US));
    }
    circuit_destroy(r.c);
}

static void gifr_case(void) {
    Rig r;
    size_t sw;

    rig_new(&r, TINY85, "t85_gifr");
    sw = rig_two_pin(&r, "AS11AH", rig_net(&r, "PB2"), r.n_vcc);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 3 * MS);
    check(is(&r, "PB1", CIRCUIT_0) && is(&r, "PB0", CIRCUIT_0),
          "gifr (the program waits for PB2)");
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, MS);
    check(is(&r, "PB1", CIRCUIT_1) && is(&r, "PB0", CIRCUIT_0) && is(&r, "PB4", CIRCUIT_0),
          "gifr (INTF0 written 1 in GIFR is cleared: the handler does not run)");
    circuit_set_switch(r.c, sw, 1, 0);
    circuit_advance(r.c, MS);
    circuit_set_switch(r.c, sw, 1, 1);
    circuit_advance(r.c, MS);
    check(is(&r, "PB4", CIRCUIT_1) && is(&r, "PB0", CIRCUIT_0),
          "int0 (the next rising edge of PB2 runs the handler)");
    circuit_destroy(r.c);
}

static void adc_case(void) {
    Rig r;
    size_t sw;

    rig_new(&r, TINY85, "t85_adc");
    sw = rig_two_pin(&r, "AS11AH", rig_net(&r, "PB2"), r.n_vcc);
    if (rig_start(&r) != 0) {
        return;
    }
    circuit_set_switch(r.c, sw, 1, 1);
    rig_to(&r, 5 * MS);
    check(is(&r, "PB4", CIRCUIT_1) && is(&r, "PB0", CIRCUIT_1) && is(&r, "PB1", CIRCUIT_0),
          "adc (ADC1 is PB2, at 5 V: full scale; ADC3 is PB3, open: 0)");
    circuit_destroy(r.c);
}

/* ---- The USI ---------------------------------------------------------------------- */

static void usi_spi_case(void) {
    Rig r;
    size_t eeprom, sck;
    uint32_t values[3] = {0}, known[3] = {0};

    rig_new(&r, TINY85, "t85_usi_spi");
    eeprom = add_part(&r, "AT25320B-SSHL-B");
    circuit_connect(r.c, eeprom, "1", rig_net(&r, "PB3"));   /* CS# */
    circuit_connect(r.c, eeprom, "2", rig_net(&r, "PB0"));   /* SO to DI */
    circuit_connect(r.c, eeprom, "3", r.n_vcc);              /* WP# */
    circuit_connect(r.c, eeprom, "4", r.n_gnd);
    circuit_connect(r.c, eeprom, "5", rig_net(&r, "PB1"));   /* SI from DO */
    circuit_connect(r.c, eeprom, "6", rig_net(&r, "PB2"));   /* SCK from USCK */
    circuit_connect(r.c, eeprom, "7", r.n_vcc);              /* HOLD# */
    circuit_connect(r.c, eeprom, "8", r.n_vcc);
    sck = rig_net(&r, "PB2");
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, 30 * MS);
    check(circuit_memory_read(r.c, eeprom, 0x10, 3, values, known) == 3 && values[0] == 0xa1 &&
              values[1] == 0xb2 && values[2] == 0xc3 && circuit_error(r.c) == NULL,
          "usi_spi (three-wire mode: A1 B2 C3 written at 0x0010 of an SPI EEPROM)");
    check(is(&r, "PB4", CIRCUIT_1) && is(&r, "PB3", CIRCUIT_1),
          "usi_spi (and read back through DI: USIDR and USIBR hold each byte)");
    check(circuit_net_value(r.c, sck) == CIRCUIT_0 && circuit_net_changes(r.c, sck) % 16 == 1,
          "usi_spi (USCK idles low, 16 edges a byte after its Z to 0)");
    if (!is(&r, "PB4", CIRCUIT_1)) {
        printf("     the EEPROM holds %02x %02x %02x; USCK changed %llu times\n", (unsigned)values[0],
               (unsigned)values[1], (unsigned)values[2],
               (unsigned long long)circuit_net_changes(r.c, sck));
    }
    circuit_destroy(r.c);
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

static void usi_i2c_case(void) {
    Rig r;
    size_t rtc, sda, scl;

    rig_new(&r, TINY85, "t85_usi_i2c");
    sda = rig_net(&r, "PB0");
    scl = rig_net(&r, "PB2");
    rtc = add_part(&r, "PCF8563T/5,518");
    circuit_connect(r.c, rtc, "4", r.n_gnd);   /* VSS */
    circuit_connect(r.c, rtc, "5", sda);       /* SDA */
    circuit_connect(r.c, rtc, "6", scl);       /* SCL */
    circuit_connect(r.c, rtc, "8", r.n_vcc);   /* VDD */
    /* Two-wire mode has no pull-ups of its own: the bus needs resistors. */
    rig_two_pin(&r, "CRCW02014K70FNED", sda, r.n_vcc);
    rig_two_pin(&r, "CRCW02014K70FNED", scl, r.n_vcc);
    if (rig_start(&r) != 0) {
        return;
    }
    rig_to(&r, MS / 2);
    check(circuit_net_value(r.c, sda) == CIRCUIT_1 && circuit_net_value(r.c, scl) == CIRCUIT_1,
          "usi_i2c (before the chip runs the resistors hold SDA and SCL at 1)");
    rig_to(&r, 30 * MS);
    check((rtc_register(&r, rtc, 2) & 0x7f) == 0x25 && circuit_error(r.c) == NULL,
          "usi_i2c (two-wire mode: 0x25 written to the clock's seconds register)");
    check(is(&r, "PB4", CIRCUIT_1) && is(&r, "PB3", CIRCUIT_0),
          "usi_i2c (read back, and every byte was acknowledged)");
    check(circuit_net_value(r.c, sda) == CIRCUIT_1 && circuit_net_value(r.c, scl) == CIRCUIT_1,
          "usi_i2c (the bus is released after the stop)");
    if (!is(&r, "PB4", CIRCUIT_1)) {
        printf("     the register %02x, PB3 %d\n", (unsigned)rtc_register(&r, rtc, 2),
               (int)circuit_net_value(r.c, rig_net(&r, "PB3")));
    }
    circuit_destroy(r.c);
}

int main(void) {
    library = part_library_create();
    if (library == NULL || load_parts_dir(library, PARTS_DIR) != 0) {
        printf("FAIL: cannot load the part library\n");
        return 1;
    }
    fact_cases();
    blink_cases();
    crystal_case();
    reset_case();
    pwm_case();
    gifr_case();
    adc_case();
    usi_spi_case();
    usi_i2c_case();
    part_library_destroy(library);
    if (failures > 0) {
        printf("FAIL: %d\n", failures);
        return 1;
    }
    printf("avr_tiny_test: all passed\n");
    return 0;
}
