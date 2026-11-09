#ifndef CONVERTER_STATE_H
#define CONVERTER_STATE_H

/*
 * Register and clock snapshots of converter and rtc parts (plan-24-I), for
 * the app's register window: circuit_converter_registers and
 * circuit_rtc_time (circuit.h).
 */

#include <stdint.h>

/* One register: its name and bus address as the datasheet gives them, the
   address space (0 unless the part has more than one), width in bits and
   current value. address is -1 for an internal register (no bus address). */
typedef struct ConverterRegister {
    const char *name;
    int32_t address;
    int space;
    unsigned width;
    uint32_t value;
} ConverterRegister;

/* An RTC's time counters now: the clock's own count, not its registers
   (which a latch may hold). hour is 0-23; weekday 0 = Sunday; hundredths
   is 1 when the part counts hundredths (else hundredth stays 0). running is
   0 while the oscillator is stopped or counting is halted. */
typedef struct RtcTime {
    int year, month, day, weekday, hour, minute, second, hundredth;
    int hundredths;
    int running;
} RtcTime;

#endif
