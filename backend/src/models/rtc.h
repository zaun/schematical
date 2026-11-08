#ifndef RTC_H
#define RTC_H

/*
 * The rtc kind (plan-24-F): the converter engine's registers and protocol
 * plus timekeeping from simulated time (planning/plan-24-data/schema.md
 * § 10): calendar fields in BCD or binary, 12/24-hour mode, the oscillator
 * stop bit, update/latch mechanisms, alarms, periodic interrupts, a
 * watchdog and battery-backed RAM. Power-on time 2000-01-01 00:00:00
 * (a Saturday). No wall-clock time: traces stay deterministic.
 */

#include "converter.h"

typedef enum {
    RU_HUNDREDTH, RU_SECOND, RU_MINUTE, RU_HOUR, RU_WEEKDAY, RU_DATE, RU_MONTH, RU_YEAR,
    RU_CENTURY, RU_COUNT
} RtcUnit;

/* binary when fixed_binary, or when the format field equals binary_value */
typedef struct {
    int fixed_binary;
    int has_field;
    CvField field;
    uint32_t binary_value;
} RtcFormat;

typedef struct {
    CvField field;
    int unit;
    RtcFormat format;
    int has_h12, has_pm;
    CvField h12, pm;
    int h12_level;
    int first;
    int base;
} RtcTimeField;

typedef struct {
    CvField field;
    int unit;
    int has_mask;
    CvField mask;
    int mask_level;
    int has_dont_care;
    uint32_t dc_mask, dc_value;
} RtcMatch;

typedef struct {
    int pin;                /* name id, CV_NONE */
    int active;
    int open_drain;
} RtcPin;

typedef struct {
    const RtcMatch *match;
    size_t match_count;
    int has_flag, has_enable;
    CvField flag, enable;
    RtcPin pin;
} RtcAlarm;

typedef struct {
    int fixed;              /* one fixed period (array form) */
    uint64_t period_ps;
    CvField rate;
    const uint32_t *keys;
    const uint64_t *ps;
    size_t count;
    int has_flag, has_enable, has_summary;
    CvField flag, enable, summary;
    RtcPin pin;
} RtcPeriodic;

typedef struct {
    int present;
    CvField field;
    const uint32_t *keys;
    const uint64_t *ps;
    size_t count;
    int has_kick;
    int kick_pin;
    int kick_edge;          /* 1 rise, 0 fall, 2 both */
    int has_flag;
    CvField flag;
    RtcPin pin;
    RtcPin reset_pin;
    const uint32_t *pulse_keys;
    const uint64_t *pulse_ps;
    size_t pulse_count;
} RtcWatchdog;

enum { RTC_UPDATE_NONE, RTC_UPDATE_UIP, RTC_UPDATE_LATCH };

typedef struct {
    int kind;
    CvField uip, inhibit;
    int has_inhibit;
    CvField latch, write;
    int has_write;
    int write_halts;
} RtcUpdate;

typedef struct RtcClock {
    const RtcTimeField *fields;
    size_t field_count;
    int has_stop;
    CvField stop;
    int stop_running;       /* stop_value is the running value (else the stopped one) */
    uint32_t stop_value;
    RtcUpdate update;
    const RtcAlarm *alarms;
    size_t alarm_count;
    const RtcPeriodic *periodic;
    size_t periodic_count;
    RtcWatchdog watchdog;
    int hundredths;         /* a hundredth field: ticks every 10 ms */
} RtcClock;

#endif
