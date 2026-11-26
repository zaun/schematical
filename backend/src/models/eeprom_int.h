/* models/eeprom_int.h: what the pieces of models/eeprom.c share (plan-68-I/J split it). */
#ifndef MODELS_EEPROM_INT_H
#define MODELS_EEPROM_INT_H

/*
 * eeprom: parallel EEPROMs (Microchip AT28 family, plan-04-A section 5).
 *
 * Memory: every instance has its own byte array, created erased (every byte
 * FF). Each bit is 0, 1 or X.
 *
 * Reads follow the SRAM read window (sram.c) with no low-Z delay: with CE#
 * and OE# low and WE# high the outputs drive X, then data at the latest of
 * address + tACC, CE# + tCE and OE# + tOE; old data holds tOH after an
 * address change; they return to Z tDF after the read ends.
 *
 * Writes follow the datasheets. A load starts when CE# and WE# are both low
 * with OE# high (the address is latched then) and ends when that stops (the
 * data is latched then). Loads within tBLC of each other form one window,
 * which may only touch one page; tBLC after the last load the internal write
 * cycle starts and runs for tWC. During it loads are ignored and reads
 * return DATA# polling (I/O7 = the complement of the last loaded byte's
 * bit 7) and the toggle bit (I/O6), with I/O0-5 X. A timing violation stores
 * the byte as X.
 *
 * Software data protection (SDP): a window that starts with the enable
 * prefix (A/AA, B/55, A/A0) turns protection on; the disable sequence
 * (A/AA, B/55, A/80, A/AA, B/55, A/20) turns it off, on devices whose
 * protection is "off" at power-up. Command bytes are never data. While
 * protection is on, a window without either sequence writes nothing (its
 * cycle still runs).
 *
 * Not modeled: 12 V chip erase and device ID, the power-on write inhibit,
 * the write-pulse noise filter, tCS and tCH (zero on every part).
 */

#include <stdlib.h>
#include <string.h>

#include "kind.h"

#define EE_NO_PIN ((size_t)-1)
#define EE_MAX_ADDRESS_BITS 24
#define EE_DATA_BITS 8
#define EE_NEVER UINT64_MAX
#define EE_MAX_UNKNOWN 32
#define EE_COMMAND_MAX 6
#define EE_COUNT(array) (sizeof(array) / sizeof((array)[0]))

enum { TIMER_REFRESH, TIMER_WINDOW, TIMER_CYCLE };

/* One speed grade. Times in picoseconds; 0 where the datasheet gives none. */
typedef struct {
    SimTime tacc, tce, toe, tdf, toh;
    SimTime tas, tah, tcs, tch, twp, twph, tds, tdh, toes, toeh, tblc, twc;
} EepromTiming;

typedef enum { PROTECTION_OFF, PROTECTION_ALWAYS } Protection;

/* ---- Instance state ------------------------------------------------------ */

typedef enum { OUT_Z, OUT_X, OUT_DATA, OUT_HOLD } OutKind;

typedef enum { MODE_IDLE, MODE_LOADING, MODE_BUSY } Mode;

/* How the window's first loads compare with the SDP sequences. */
typedef enum {
    COMMAND_MATCHING, /* every load so far is a step of a sequence */
    COMMAND_NONE,     /* the window is data */
    COMMAND_ENABLE,   /* the enable prefix is done; the rest is data */
    COMMAND_DISABLE   /* the disable sequence is done; the rest is data */
} CommandState;

/* Where the last load went, so a hold violation can turn it X. */
typedef enum { LAST_NONE, LAST_BUFFER, LAST_COMMAND } LastTarget;

typedef struct {
    uint32_t address;
    uint8_t value;
    uint8_t known;
} Load;

/* A write to every word matching the known address bits, stored as X. */
typedef struct {
    uint32_t address;
    uint32_t known;
} Unknown;

/* An input control: 1 active, 0 inactive, -1 unknown. */
typedef struct {
    int level;
    SimTime active_since;
    SimTime inactive_since;
    SimTime changed;
} Control;

typedef struct {
    const EepromTiming *t;
    unsigned address_bits;
    uint32_t word_count;
    uint32_t page_size;
    uint32_t command_a;
    uint32_t command_b;
    Protection protection;

    size_t addr_pin[EE_MAX_ADDRESS_BITS];
    size_t dq_pin[EE_DATA_BITS];
    size_t ce_pin;
    size_t oe_pin;
    size_t we_pin;

    /* Committed contents. */
    uint8_t *value;
    uint8_t *known;

    /* Inputs as last seen, with the value before the latest change time. */
    LogicValue addr[EE_MAX_ADDRESS_BITS];
    LogicValue addr_prev[EE_MAX_ADDRESS_BITS];
    SimTime addr_time;
    SimTime addr_prev_time;
    LogicValue dq[EE_DATA_BITS];
    LogicValue dq_prev[EE_DATA_BITS];
    SimTime dq_time[EE_DATA_BITS];
    SimTime dq_prev_time[EE_DATA_BITS];
    Control ce;
    Control oe;
    Control we;

    /* Read side. */
    int out_enable;          /* read_on as last evaluated: 1, 0 or -1 */
    SimTime off_time;
    int off_from_z;
    OutKind out_kind;
    SimTime hold_until;
    LogicValue hold[EE_DATA_BITS];
    LogicValue driven[EE_DATA_BITS];
    uint8_t poll_value;      /* what a read during the write cycle returns */
    uint8_t poll_known;
    unsigned poll_reads;
    SimTime wake;

    /* Write side. */
    int writing;             /* write_on as last evaluated: 1, 0 or -1 */
    int load_active;         /* a load started outside the write cycle */
    SimTime load_start;
    uint32_t load_addr;
    uint32_t load_addr_known;
    int load_bad;

    Mode mode;
    int protected_on;
    SimTime last_end;        /* end of the window's latest load */
    SimTime cycle_end;
    int writes_allowed;      /* set when the cycle starts */

    /* The window. */
    CommandState command_state;
    Load command[EE_COMMAND_MAX];
    unsigned command_count;
    int has_page;
    uint32_t page_base;
    int mixed;
    uint8_t *buf_value;      /* page_size each */
    uint8_t *buf_known;
    uint8_t *buf_loaded;
    Unknown unknown[EE_MAX_UNKNOWN];
    unsigned unknown_count;

    /* The latest load, for DATA# polling and the tDH / tOEH checks. */
    uint8_t last_value;
    uint8_t last_known;
    int last_hold_valid;
    SimTime last_hold_end;
    LastTarget last_target;
    uint32_t last_index;
} EepromState;

typedef struct {
    EepromTiming timing;
    PartEeprom info;
    const PartVoltage *vcc;   /* the grade's "vcc"/"inputMax"; NULL = none */
} EepromGrade;

typedef struct {
    const char *const *address;
    size_t address_count;
    const char *const *data;
    size_t data_count;
    const char *ce;
    const char *oe;
    const char *we;
    uint32_t page_size;
    uint32_t command[2];
    Protection protection;
    const char **grade_names;
    EepromGrade *grades;
    size_t grade_count;
} EepromDevice;

typedef struct {
    size_t addr[EE_MAX_ADDRESS_BITS];
    size_t dq[EE_DATA_BITS];
    size_t ce;
    size_t oe;
    size_t we;
    const EepromDevice *device;
} EepromPins;

void eep_eeprom_start(Sim *sim, SimInstance *instance, void *raw);
void eep_eeprom_pin_changed(Sim *sim, SimInstance *instance, void *state, size_t pin,
                               LogicValue old_value, LogicValue new_value);
void eep_eeprom_timer(Sim *sim, SimInstance *instance, void *state, uint32_t timer_id);
int eep_eeprom_memory_size(const void *raw, uint32_t *words, unsigned *bits);
size_t eep_eeprom_memory_read(const void *raw, uint32_t start, size_t count,
                                 uint32_t *values, uint32_t *known);
void eep_eeprom_memory_load(void *raw, uint32_t start, const uint32_t *values, size_t count);

#endif
