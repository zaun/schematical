#ifndef SIM_H
#define SIM_H

#include <stddef.h>
#include <stdint.h>

typedef uint64_t SimTime;

typedef enum {
    LOGIC_0,
    LOGIC_1,
    LOGIC_X,
    LOGIC_Z
} LogicValue;

typedef enum {
    DRIVE_WEAK,
    DRIVE_STRONG
} DriveStrength;

typedef enum {
    PACKAGE_TYPE_SOIC,
    PACKAGE_TYPE_SOP,
    PACKAGE_TYPE_SSOP,
    PACKAGE_TYPE_DIP,
    PACKAGE_TYPE_TSSOP,
    PACKAGE_TYPE_TVSOP,
    PACKAGE_TYPE_CDIP,
    PACKAGE_TYPE_CFP,
    PACKAGE_TYPE_LCCC,
    PACKAGE_TYPE_WQFN,
    PACKAGE_TYPE_VQFN,
    PACKAGE_TYPE_VSSOP,
    PACKAGE_TYPE_SOT23,
    PACKAGE_TYPE_SC70,
    PACKAGE_TYPE_DSBGA,
    PACKAGE_TYPE_SON,
    PACKAGE_TYPE_UQFN,
    PACKAGE_TYPE_X2SON,
    PACKAGE_TYPE_SOT5X3,
    PACKAGE_TYPE_SOJ,
    PACKAGE_TYPE_TSOP_I,
    PACKAGE_TYPE_STSOP_I,   /* small TSOP type I, 8 x 13.4 mm */
    PACKAGE_TYPE_TSOP_II,
    PACKAGE_TYPE_TFBGA,
    PACKAGE_TYPE_SMD,       /* surface-mount switch (switch.c) */
} PackageType;

typedef enum {
    LOGIC_FAMILY_HC,
    LOGIC_FAMILY_LS,
    LOGIC_FAMILY_ACT,
    LOGIC_FAMILY_HCT,
    LOGIC_FAMILY_F,
    LOGIC_FAMILY_AC,
    LOGIC_FAMILY_AHCT,
    LOGIC_FAMILY_ALS,
    LOGIC_FAMILY_AS,
    LOGIC_FAMILY_AHC,
    LOGIC_FAMILY_S,
    LOGIC_FAMILY_C,
    LOGIC_FAMILY_H,
    LOGIC_FAMILY_HCS,
    LOGIC_FAMILY_LV,
    LOGIC_FAMILY_LVC,
    LOGIC_FAMILY_AUP,
    LOGIC_FAMILY_AUC,
    LOGIC_FAMILY_ALVC,
    LOGIC_FAMILY_TTL,   /* standard TTL (5476, 54107) */
    LOGIC_FAMILY_SRAM,  /* asynchronous SRAM (sram.c) */
    LOGIC_FAMILY_SWITCH,/* switch (switch.c) */
    LOGIC_FAMILY_LED7   /* LED 7-segment display (led7.c) */
} LogicFamily;

typedef struct {
    PackageType package;
    LogicFamily family;
    /*
     * Model data for models that need more than package and family, such as
     * an SRAM's pinout and speed grade. Points to static tables; NULL for
     * models that do not use it.
     */
    const void *model_data;
    size_t variant;
} SimLogicConfig;

typedef struct Sim Sim;
typedef struct SimInstance SimInstance;
struct CpuState;
struct LcdState;
struct ConverterRegister;
struct RtcTime;
typedef struct SimNet SimNet;

typedef struct {
    const char *name;

    size_t (*pin_count)(const void *config);
    void *(*create)(const void *config);
    void (*destroy)(void *state);

    void (*start)(Sim *sim, SimInstance *instance, void *state);

    void (*pin_changed)(
        Sim *sim,
        SimInstance *instance,
        void *state,
        size_t pin,
        LogicValue old_value,
        LogicValue new_value
    );

    void (*timer)(
        Sim *sim,
        SimInstance *instance,
        void *state,
        uint32_t timer_id
    );

    /*
     * Optional hooks for the part library's models. All may be NULL.
     *
     * lights: writes 1 per light (LED, display segment) that is on now or
     * was on at any time at or after `since`, else 0; at most cap values.
     * Returns the number of lights.
     */
    size_t (*lights)(const Sim *sim, const SimInstance *instance, const void *state,
                     SimTime since, unsigned char *out, size_t cap);

    /* Switches: turns a position (1-based) on or off. 1 on success, 0 if out of range. */
    int (*set_switch)(Sim *sim, SimInstance *instance, void *state, size_t position, int on);

    /* Memories: word count and width. 1 on success. */
    int (*memory_size)(const void *state, uint32_t *words, unsigned *bits);

    /*
     * Memories: copies count words from start; values gets the bits, known a
     * 1 per defined bit. Either may be NULL. Returns the number copied.
     */
    size_t (*memory_read)(const void *state, uint32_t start, size_t count,
                          uint32_t *values, uint32_t *known);

    /*
     * light_time: writes, per light (same lights and order as `lights`),
     * the cumulative picoseconds it has been on since start, at sim_now; at
     * most cap values. Returns the number of lights.
     */
    size_t (*light_time)(const Sim *sim, const SimInstance *instance, const void *state,
                         uint64_t *out, size_t cap);
    /*
     * Memories: stores count words from start, every bit known. The caller
     * has checked the range and widths. NULL = not loadable.
     */
    void (*memory_load)(void *state, uint32_t start, const uint32_t *values, size_t count);
    /* Processors: fills out with the processor's state (cpu_state.h). 1 when filled. */
    int (*cpu_state)(const void *state, struct CpuState *out);
    /* Character LCDs: fills out with the display's state (lcd_state.h) at
       sim_now (the cursor blink phase depends on it). 1 when filled. */
    int (*lcd_state)(const Sim *sim, const void *state, struct LcdState *out);
    /* Serial parts (plan-12-C section 3.4): number of channels (0 = not a serial part). */
    size_t (*serial_channels)(const void *state);
    /* Frames the channel has sent since start: the total count, and a copy of
       frames [start, start + cap). Each frame is its data bits, LSB = bit 0. */
    size_t (*serial_sent)(const void *state, unsigned channel, size_t start,
                          uint16_t *frames, size_t cap);
    /* Queues bytes to drive onto the channel's RX pin (plan-12-A section 3.7).
       1 when queued; 0 when the channel is not in an asynchronous mode. */
    int (*serial_send)(Sim *sim, SimInstance *instance, void *state, unsigned channel,
                       const uint8_t *bytes, size_t count);
    /* Register parts (converter, rtc; plan-24-I): the register count, and
       copies of registers [0, cap) into out (converter_state.h). */
    size_t (*registers)(const void *state, struct ConverterRegister *out, size_t cap);
    /* RTCs: fills out with the clock's counters now. 1 when filled. */
    int (*rtc_time)(const void *state, struct RtcTime *out);
} SimComponentType;

/* All times are in picoseconds. */
Sim *sim_create(void);
void sim_destroy(Sim *sim);

const char *sim_error(const Sim *sim);

/* The net that blew up, or NULL. Set together with the "smoke:" error. */
const SimNet *sim_shorted_net(const Sim *sim);
SimTime sim_now(const Sim *sim);

/* The model must copy any config data it needs after this call. */
SimInstance *sim_add_instance(
    Sim *sim,
    const SimComponentType *type,
    const char *name,
    const void *config
);

SimNet *sim_add_net(Sim *sim, const char *name);

/* The model type and model state of an instance, so models can offer inspection. */
const SimComponentType *sim_instance_type(const SimInstance *instance);
void *sim_instance_state(const SimInstance *instance);

int sim_connect(
    Sim *sim,
    SimNet *net,
    SimInstance *instance,
    size_t pin
);

int sim_start(Sim *sim);
int sim_run_until(Sim *sim, SimTime time);

LogicValue sim_read_pin(
    const Sim *sim,
    const SimInstance *instance,
    size_t pin
);

void sim_schedule_drive(
    Sim *sim,
    SimInstance *instance,
    size_t pin,
    LogicValue value,
    DriveStrength strength,
    SimTime delay
);

/*
 * Like sim_schedule_drive, but a newer drive on the same pin cancels any drive
 * still pending on it. Gate outputs use this so pulses shorter than the
 * propagation delay are filtered out. A drive of the value already pending on
 * the pin, no earlier than it, keeps the pending one.
 */
void sim_schedule_inertial_drive(
    Sim *sim,
    SimInstance *instance,
    size_t pin,
    LogicValue value,
    DriveStrength strength,
    SimTime delay
);

/*
 * Contacts, for switches. A contact joins two pins of one instance. While it
 * is closed, the nets on those pins act as one net: every driver on either
 * side counts, so a value passes through in both directions, and strong
 * drivers that disagree across it short. Chains and loops of contacts work
 * the same way. A pin that is not connected to a net is left out.
 *
 * Before sim_start the state is set at once. Once running, the change takes
 * effect at the current time, on the next sim_run_until. Contacts start open.
 */
void sim_set_contact(
    Sim *sim,
    SimInstance *instance,
    size_t pin_a,
    size_t pin_b,
    int closed
);

/* Whether the contact between two pins of an instance is closed. */
int sim_contact_closed(
    const Sim *sim,
    const SimInstance *instance,
    size_t pin_a,
    size_t pin_b
);

void sim_schedule_timer(
    Sim *sim,
    SimInstance *instance,
    uint32_t timer_id,
    SimTime delay
);

#endif
