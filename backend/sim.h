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
    PACKAGE_TYPE_PDIP,
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
    LOGIC_FAMILY_AUC
} LogicFamily;

typedef struct {
    PackageType package;
    LogicFamily family;
} SimLogicConfig;

typedef struct Sim Sim;
typedef struct SimInstance SimInstance;
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
 * propagation delay are filtered out.
 */
void sim_schedule_inertial_drive(
    Sim *sim,
    SimInstance *instance,
    size_t pin,
    LogicValue value,
    DriveStrength strength,
    SimTime delay
);

void sim_schedule_timer(
    Sim *sim,
    SimInstance *instance,
    uint32_t timer_id,
    SimTime delay
);

#endif
