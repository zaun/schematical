/*
 * Host bus helpers (serial_bus.h; plan-12-C section 3.3).
 */

#include "serial_bus.h"

#include <string.h>

LogicValue serial_level(const Sim *sim, const SimInstance *instance, size_t pin, LogicValue z_reads) {
    LogicValue value;

    if (pin == SERIAL_NO_PIN) {
        return z_reads;
    }
    value = sim_read_pin(sim, instance, pin);
    if (value == LOGIC_Z) {
        return z_reads;
    }
    return value == LOGIC_0 || value == LOGIC_1 ? value : LOGIC_X;
}

void serial_bus_init(SerialBus *bus, const SerialBusPins *pins, const SerialBusTiming *timing) {
    size_t i;

    memset(bus, 0, sizeof *bus);
    bus->pins = pins;
    bus->timing = *timing;
    for (i = 0; i < SERIAL_BUS_ADDRESS; i++) {
        bus->latched_address[i] = LOGIC_X;
    }
    bus->latched_select = LOGIC_0;
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        bus->data_out[i] = LOGIC_Z;
    }
    bus->ddis_out = LOGIC_Z;
    bus->select_out = LOGIC_Z;
    bus->enabled = 1;
}

int serial_bus_has_pin(const SerialBus *bus, size_t pin) {
    const SerialBusPins *p = bus->pins;
    unsigned i;

    if (pin == SERIAL_NO_PIN) {
        return 0;
    }
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        if (p->data[i] == pin) {
            return 1;
        }
    }
    for (i = 0; i < p->address_count; i++) {
        if (p->address[i] == pin) {
            return 1;
        }
    }
    for (i = 0; i < p->select_count; i++) {
        if (p->select[i] == pin) {
            return 1;
        }
    }
    for (i = 0; i < p->select_high_count; i++) {
        if (p->select_high[i] == pin) {
            return 1;
        }
    }
    return pin == p->read || pin == p->read_high || pin == p->write || pin == p->write_high ||
           pin == p->address_strobe;
}

/* AND of the select pins: 0, 1 or X (Z reads inactive). */
static LogicValue pin_select(const SerialBus *bus, const Sim *sim, const SimInstance *instance) {
    const SerialBusPins *p = bus->pins;
    int unknown = 0;
    unsigned i;

    for (i = 0; i < p->select_count; i++) {
        LogicValue v = serial_level(sim, instance, p->select[i], LOGIC_1);

        if (v == LOGIC_1) {
            return LOGIC_0;
        }
        unknown |= v == LOGIC_X;
    }
    for (i = 0; i < p->select_high_count; i++) {
        LogicValue v = serial_level(sim, instance, p->select_high[i], LOGIC_0);

        if (v == LOGIC_0) {
            return LOGIC_0;
        }
        unknown |= v == LOGIC_X;
    }
    return unknown ? LOGIC_X : LOGIC_1;
}

static void address_of(const SerialBus *bus, unsigned *value, unsigned *known) {
    unsigned i;

    *value = 0;
    *known = 0;
    for (i = 0; i < bus->pins->address_count; i++) {
        if (bus->latched_address[i] == LOGIC_1) {
            *value |= 1u << i;
        }
        if (bus->latched_address[i] != LOGIC_X) {
            *known |= 1u << i;
        }
    }
}

/* Follows ADS: transparent while low (or absent), latched on its rising edge. */
static void track_latch(SerialBus *bus, const Sim *sim, const SimInstance *instance,
                        const SerialBusEvents *events) {
    const SerialBusPins *p = bus->pins;
    LogicValue ads = p->address_strobe == SERIAL_NO_PIN
        ? LOGIC_0 : serial_level(sim, instance, p->address_strobe, LOGIC_X);
    unsigned value;
    unsigned known;
    unsigned i;

    if (ads == LOGIC_1) {
        return;   /* held */
    }
    for (i = 0; i < p->address_count; i++) {
        bus->latched_address[i] = ads == LOGIC_X ? LOGIC_X
                                                 : serial_level(sim, instance, p->address[i], LOGIC_X);
    }
    address_of(bus, &value, &known);
    bus->latched_unit = 0;
    if (ads == LOGIC_X) {
        bus->latched_select = LOGIC_X;
    } else if (events->select != NULL) {
        bus->latched_select = events->select(events->context, sim, instance, value, known,
                                             &bus->latched_unit);
    } else {
        bus->latched_select = pin_select(bus, sim, instance);
    }
}

/* Two strobes, one active-low and one active-high: 0 none, 1 one, 2 both, 3 X. */
static int strobe_state(const Sim *sim, const SimInstance *instance, size_t low, size_t high) {
    LogicValue a = low == SERIAL_NO_PIN ? LOGIC_1 : serial_level(sim, instance, low, LOGIC_X);
    LogicValue b = high == SERIAL_NO_PIN ? LOGIC_0 : serial_level(sim, instance, high, LOGIC_X);

    if (a == LOGIC_X || b == LOGIC_X) {
        return 3;
    }
    return (a == LOGIC_0) + (b == LOGIC_1);
}

static void drive_pin(Sim *sim, SimInstance *instance, size_t pin, LogicValue *current,
                      LogicValue value, SimTime delay) {
    if (pin == SERIAL_NO_PIN || *current == value) {
        return;
    }
    *current = value;
    sim_schedule_inertial_drive(sim, instance, pin, value, DRIVE_STRONG, delay);
}

static void drive_data(SerialBus *bus, Sim *sim, SimInstance *instance, uint8_t data, uint8_t known,
                       int release, SimTime delay) {
    unsigned i;

    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        LogicValue v = release ? LOGIC_Z
                     : !((known >> i) & 1u) ? LOGIC_X
                     : (data >> i) & 1u ? LOGIC_1 : LOGIC_0;

        drive_pin(sim, instance, bus->pins->data[i], &bus->data_out[i], v, delay);
    }
}

static void read_data(const SerialBus *bus, const Sim *sim, const SimInstance *instance,
                      uint8_t *data, uint8_t *known) {
    unsigned i;

    *data = 0;
    *known = 0;
    for (i = 0; i < SERIAL_BUS_DATA; i++) {
        LogicValue v = serial_level(sim, instance, bus->pins->data[i], LOGIC_X);

        if (v == LOGIC_1) {
            *data |= (uint8_t)(1u << i);
        }
        if (v != LOGIC_X) {
            *known |= (uint8_t)(1u << i);
        }
    }
}

static void drive_read(SerialBus *bus, Sim *sim, SimInstance *instance,
                       const SerialBusEvents *events) {
    uint8_t data = 0;
    uint8_t known = 0;

    if (bus->read == SERIAL_CYCLE_ACTIVE) {
        events->peek(events->context, bus->read_unit, bus->read_address, &data, &known);
    }
    drive_data(bus, sim, instance, data, known, 0, bus->timing.access);
}

/* Every register an address with unknown bits could name gets X. */
static void write_unknown(const SerialBus *bus, const SerialBusEvents *events, unsigned unit,
                          unsigned address, unsigned known) {
    unsigned count = 1u << bus->pins->address_count;
    unsigned a;

    for (a = 0; a < count; a++) {
        if ((a & known) == (address & known)) {
            events->write(events->context, unit, a, 0, 0);
        }
    }
}

void serial_bus_start(SerialBus *bus, Sim *sim, SimInstance *instance,
                      const SerialBusEvents *events) {
    drive_data(bus, sim, instance, 0, 0, 1, 0);
    drive_pin(sim, instance, bus->pins->driver_disable, &bus->ddis_out, LOGIC_1, 0);
    serial_bus_update(bus, sim, instance, events);
}

void serial_bus_update(SerialBus *bus, Sim *sim, SimInstance *instance,
                       const SerialBusEvents *events) {
    const SerialBusPins *p = bus->pins;
    unsigned address;
    unsigned known;
    unsigned full = (1u << p->address_count) - 1;
    int rd;
    int wr;
    LogicValue sel;
    SerialCycle read;
    SerialCycle write;

    track_latch(bus, sim, instance, events);
    address_of(bus, &address, &known);
    sel = bus->enabled ? bus->latched_select : LOGIC_0;
    drive_pin(sim, instance, p->select_out, &bus->select_out, sel, bus->timing.select_out);

    rd = strobe_state(sim, instance, p->read, p->read_high);
    wr = strobe_state(sim, instance, p->write, p->write_high);

    /* Read cycle. */
    if (sel == LOGIC_0 || rd == 0) {
        read = SERIAL_CYCLE_NONE;
    } else if (sel == LOGIC_1 && rd == 1 && known == full) {
        read = SERIAL_CYCLE_ACTIVE;
    } else {
        read = SERIAL_CYCLE_X;
    }
    if (read != bus->read || (read == SERIAL_CYCLE_ACTIVE &&
                              (address != bus->read_address || bus->latched_unit != bus->read_unit))) {
        if (bus->read == SERIAL_CYCLE_ACTIVE && read == SERIAL_CYCLE_NONE) {
            events->read_done(events->context, bus->read_unit, bus->read_address);
        }
        bus->read = read;
        bus->read_address = address;
        bus->read_unit = bus->latched_unit;
        if (read == SERIAL_CYCLE_NONE) {
            drive_data(bus, sim, instance, 0, 0, 1, bus->timing.release);
            drive_pin(sim, instance, p->driver_disable, &bus->ddis_out, LOGIC_1, bus->timing.ddis);
        } else {
            drive_read(bus, sim, instance, events);
            drive_pin(sim, instance, p->driver_disable, &bus->ddis_out,
                      read == SERIAL_CYCLE_ACTIVE ? LOGIC_0 : LOGIC_X, bus->timing.ddis);
        }
    }

    /* Write cycle: stored when it ends. */
    if (sel == LOGIC_0 || wr == 0) {
        write = SERIAL_CYCLE_NONE;
    } else if (sel == LOGIC_1 && (wr == 1 || wr == 2) && known == full) {
        write = SERIAL_CYCLE_ACTIVE;
    } else {
        write = SERIAL_CYCLE_X;
    }
    if (write != bus->write) {
        if (bus->write == SERIAL_CYCLE_ACTIVE) {
            uint8_t data;
            uint8_t data_known;

            read_data(bus, sim, instance, &data, &data_known);
            if (write == SERIAL_CYCLE_X) {
                data_known = 0;
            }
            events->write(events->context, bus->write_unit, bus->write_address, data, data_known);
        } else if (bus->write == SERIAL_CYCLE_X && write == SERIAL_CYCLE_NONE) {
            write_unknown(bus, events, bus->write_unit, bus->write_address,
                          bus->write_address_known);
        }
        bus->write = write;
        bus->write_unit = bus->latched_unit;
        bus->write_address = address;
        bus->write_address_known = known;
    } else if (write == SERIAL_CYCLE_ACTIVE) {
        /* The address may move during the strobe; the last one counts. */
        bus->write_address = address;
        bus->write_unit = bus->latched_unit;
    } else if (write == SERIAL_CYCLE_X) {
        bus->write_address_known &= known & ~(address ^ bus->write_address);
    }
}

void serial_bus_refresh(SerialBus *bus, Sim *sim, SimInstance *instance,
                        const SerialBusEvents *events) {
    if (bus->read == SERIAL_CYCLE_ACTIVE) {
        drive_read(bus, sim, instance, events);
    }
}
