/*
 * The ERC pin types (include/erc.h, plan-63-B): the KiCad pin type of a
 * part's pin, from its role, its kind and the kind's own answers.
 */

#include "erc.h"

#include "model_pins.h"

/* The kinds whose every pin KiCad's own symbols type passive. */
static int kind_is_passive(PartKind kind) {
    switch (kind) {
    case PART_CONNECTOR:
    case PART_HEADER:
    case PART_SOCKET:
    case PART_RESISTOR:
    case PART_CAPACITOR:
    case PART_INDUCTOR:
    case PART_DIODE:
    case PART_LED:
    case PART_FUSE:
    case PART_CRYSTAL:
    case PART_RELAY:
    case PART_BUZZER:
    case PART_SWITCH:
    case PART_TRANSISTOR:
    case PART_7SEG:
    case PART_NIXIE:
    case PART_DEKATRON:
    case PART_LCD:
        return 1;
    default:
        return 0;
    }
}

/* 1 when pin is the output name of a regulator element or a power switch. */
static int is_supply_output(const Part *part, size_t pin) {
    size_t k;

    if (part->kind == PART_REGULATOR) {
        for (k = 0; regulator_element(part, k, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL); k++) {
            if (regulator_pin_out(part, k, pin)) {
                return 1;
            }
        }
    } else if (part->kind == PART_POWERSWITCH) {
        for (k = 0; powerswitch_element(part, k, NULL, NULL, NULL, NULL, NULL, NULL, NULL); k++) {
            if (powerswitch_pin_out(part, k, pin)) {
                return 1;
            }
        }
    }
    return 0;
}

int erc_pin_type(const Part *part, size_t pin, ErcPinType *type) {
    PinRole role;

    if (part == NULL || type == NULL || pin >= part->pin_count) {
        return 0;
    }
    role = part->pins[pin].role;
    if (role == PIN_ABSENT) {
        return 0;
    }
    if (role == PIN_NC) {
        *type = ERC_NO_CONNECT;
        return 1;
    }
    if (part->kind == PART_SUPPLY) {
        if (part->supply != NULL && part->supply->pull) {
            *type = ERC_PASSIVE;
        } else if (role == PIN_POWER || role == PIN_GROUND) {
            *type = ERC_POWER_IN;
        } else {
            *type = ERC_PASSIVE;
        }
        return 1;
    }
    if (kind_is_passive(part->kind) && (role == PIN_INPUT || role == PIN_OUTPUT || role == PIN_INOUT)) {
        *type = ERC_PASSIVE;
        return 1;
    }
    switch (role) {
    case PIN_POWER:
    case PIN_GROUND:
        *type = ERC_POWER_IN;
        break;
    case PIN_OUTPUT:
        if (is_supply_output(part, pin)) {
            *type = ERC_POWER_OUT;
        } else if (part_pin_open_drain(part, pin)) {
            *type = ERC_OPEN_COLLECTOR;
        } else if (part_pin_tristate(part, pin)) {
            *type = ERC_TRI_STATE;
        } else {
            *type = ERC_OUTPUT;
        }
        break;
    case PIN_INOUT:
        *type = part_pin_open_drain(part, pin) ? ERC_OPEN_COLLECTOR : ERC_BIDIRECTIONAL;
        break;
    default:
        *type = ERC_INPUT;
        break;
    }
    return 1;
}

int erc_pin_drives_power(const Part *part, size_t pin) {
    if (part == NULL || pin >= part->pin_count || part->kind != PART_SUPPLY || part->supply == NULL ||
        part->supply->pull) {
        return 0;
    }
    return part->pins[pin].role == PIN_POWER || part->pins[pin].role == PIN_GROUND;
}

const char *erc_pin_type_name(ErcPinType type) {
    static const char *const NAMES[ERC_PIN_TYPE_COUNT] = {
        "input", "output", "bidirectional", "tri_state", "passive", "free", "unspecified",
        "power_in", "power_out", "open_collector", "open_emitter", "no_connect",
    };

    return (unsigned)type < ERC_PIN_TYPE_COUNT ? NAMES[type] : "unspecified";
}
