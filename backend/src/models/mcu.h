#ifndef MCU_H
#define MCU_H

/*
 * The mcu kind's internal interface (plan-62-E): what the loader (mcu.c)
 * builds for a device with a "model", and the engine adapter for AVR
 * models (mcu_avr.c) that mcu.c's engine model forwards to.
 */

#include "avr_chip.h"
#include "cpu_state.h"
#include "kind.h"

#define MCU_NO_PIN ((size_t)-1)

typedef struct {
    PartMcu facts;
    const PartVoltage *vcc;
    const AvrLayout *layout;   /* NULL: no "model", facts only */
} McuDevice;

/* A model device's pins in one pinout, by name. */
typedef struct {
    const AvrLayout *layout;
    size_t port[AVR_LAYOUT_PORTS][8];   /* [layout port][bit]: the pin "P<letter><bit>" */
    size_t adc6, adc7;                  /* pins named ADC6 and ADC7 */
} McuAvrPins;

/* ---- mcu_avr.c: the engine adapter ----------------------------------------- */

typedef struct McuAvr McuAvr;

McuAvr *mcu_avr_create(const Part *part, const McuAvrPins *pins);
void mcu_avr_destroy(McuAvr *mcu);
void mcu_avr_start(McuAvr *mcu, Sim *sim, SimInstance *instance);
void mcu_avr_pin_changed(McuAvr *mcu, size_t pin, LogicValue old_value, LogicValue new_value);
void mcu_avr_timer(McuAvr *mcu, uint32_t timer_id);
int mcu_avr_memory_size(const McuAvr *mcu, uint32_t *words, unsigned *bits);
size_t mcu_avr_memory_read(const McuAvr *mcu, uint32_t start, size_t count, uint32_t *values,
                           uint32_t *known);
void mcu_avr_memory_load(McuAvr *mcu, uint32_t start, const uint32_t *values, size_t count);
int mcu_avr_cpu_state(const McuAvr *mcu, CpuState *out);

#endif
